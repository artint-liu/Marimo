#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "ArithmeticExpression.h"
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

#define FOR_EACH_MBO(_N, _IDX) for(int _IDX = 0; s_Operator##_N[_IDX].szOperator != NULL; _IDX++)

namespace UVShader
{
  ArithmeticExpression::ArithmeticExpression()
    : m_pMsg(NULL)
    //, m_nMaxPrecedence(0)
    , m_nDbgNumOfExpressionParse(0)
  {
#ifdef _DEBUG
    // 检查名字与其设定长度是一致的
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i) {
      ASSERT(GXSTRLEN(s_aIntrinsicType[i].name) == s_aIntrinsicType[i].name_len);
    }
#endif // #ifdef _DEBUG

    u32 aCharSem[128];
    GetCharSemantic(aCharSem, 0, 128);

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
    SetCharSemantic(aCharSem, 0, 128);
    SetIteratorCallBack(IteratorProc, 0);
    //SetTriggerCallBack(MultiByteOperatorProc, 0);
  }

  ArithmeticExpression::~ArithmeticExpression()
  {
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

      ASSERT(m_CurSymInfo.marker.marker == NULL ||
        m_CurSymInfo.marker.marker == it.marker); // 遍历时一定这个要保持一致

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
        auto iter_token = m_Macros.find(token.marker.ToString());
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

  const ArithmeticExpression::TokenArray* ArithmeticExpression::GetTokensArray() const
  {
    return &m_aTokens;
  }

  clsize ArithmeticExpression::EstimateForTokensCount() const
  {
    auto count = GetStreamCount();
    return (count << 1) + (count >> 1); // 按照 字节数：符号数=2.5：1来计算
  }

  const ArithmeticExpression::MBO* MatchOperator(const CodeParser::MBO* op, u32 op_len, CodeParser::iterator& it, u32 remain)
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
    GXBOOL bIsNumeric = SmartStreamUtility::ExtendToCStyleNumeric(it, remain);
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
      ASSERT(l_token.marker.marker == NULL); // 每次用完外面都要清理这个

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
      }
    }
    return 0;
  }

  ArithmeticExpression::SYNTAXNODE::FLAGS ArithmeticExpression::TryGetNodeType( const SYNTAXNODE::UN* pUnion ) const
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

  ArithmeticExpression::SYNTAXNODE::MODE ArithmeticExpression::TryGetNode( const SYNTAXNODE::UN* pUnion ) const
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

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
  {
    return MakeSyntaxNode(pDest, mode, NULL, pOperandA, pOperandB);
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
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

  GXBOOL ArithmeticExpression::MakeInstruction(const TOKEN* pOpcode, int nMinPrecedence, const RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle)
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

    MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Opcode, pOpcode, &A, &B);

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
  //////////////////////////////////////////////////////////////////////////

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE scope(begin, end);
    return ParseArithmeticExpression(scope, pUnion);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(const RTSCOPE& scope_in, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE scope = scope_in;
    if(scope.end > scope.begin && m_aTokens[scope.end - 1] == ';') {
      scope.end--;
    }
    return ParseArithmeticExpression(scope, pUnion, TOKEN::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, int nMinPrecedence)
  {
    int nCandidate = s_MaxPrecedence;
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
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Opcode, A.pSym, NULL, &B);
        DbgDumpScope(A.pSym->ToString(), RTSCOPE(0,0), RTSCOPE(scope.begin + 1, scope.end));
      }
      else if(B.pSym->precedence > 0)
      {
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Opcode, B.pSym, &A, NULL);
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
        A.pSym = &m_aTokens[scope.begin + 1];
        scope_expr.begin++;
      }
      else {
        A.pSym = &front;
      }
      B.ptr = NULL;
      GXBOOL bret = ParseArithmeticExpression(scope_expr, &B);
      bret = bret && MakeSyntaxNode(pUnion, mode, &A, &B);
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

  GXBOOL ArithmeticExpression::ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
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
        ERROR_MSG__MISSING_SEMICOLON(*A.pSym);
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

  GXBOOL ArithmeticExpression::ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
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

    MakeSyntaxNode(pUnion, mode, &A, &B);
    DbgDumpScope(bracket == '(' ? "F" : "I", RTSCOPE(scope.begin, scope.begin + 1),
      RTSCOPE(scope.begin + 2, scope.end - 1));

    return bret;
  }

  void ArithmeticExpression::MarryBracket(PairStack* sStack, TOKEN& token, int& EOE)
  {
    const int c_size = (int)m_aTokens.size();
    for(int i = 0; i < s_nPairMark; ++i)
    {
      PAIRMARK& c = s_PairMark[i];
      PairStack&    s = sStack[i];
      if(token.marker == c.chOpen) {
        s.push(c_size);
      }
      else if(token.marker == c.chClosed) {
        if(s.empty()) {
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

      if(c.bNewEOE) {
        EOE = c_size + 1;
      }
    } // for
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

} // namespace UVShader