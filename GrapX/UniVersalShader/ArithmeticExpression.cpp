#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include <clStablePool.h>
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
//  2 OPP(13)   ++   --           Suffix/postfix increment and decrement                    Left-to-right
//              ()                Function call
//              []                Array subscripting
//              .                 Element selection by reference
//
//  3 OPP(12)   ++   --           Prefix increment and decrement                            Right-to-left 
//              +   −             Unary plus and minus
//              !   ~             Logical NOT and bitwise NOT
//              (type)            Type cast
//  5  OPP(11)  *   /   %         Multiplication, division, and remainder                   Left-to-right 
//  6  OPP(10)  +   −             Addition and subtraction                                  Left-to-right 
//  7  OPP(9)   <<   >>           Bitwise left shift and right shift                        Left-to-right 
//  8  OPP(8)   <   <=            For relational operators < and ≤ respectively             Left-to-right 
//              >   >=            For relational operators > and ≥ respectively             Left-to-right 
//  9  OPP(7)   ==   !=           For relational = and ≠ respectively                       Left-to-right 
//  10 OPP(6)   &                 Bitwise AND                                               Left-to-right 
//  11 OPP(5)   ^                 Bitwise XOR (exclusive or)                                Left-to-right 
//  12 OPP(4)   |                 Bitwise OR (inclusive or)                                 Left-to-right 
//  13 OPP(3)   &&                Logical AND                                               Left-to-right 
//  14 OPP(2)   ||                Logical OR                                                Left-to-right 
//  15 OPP(1)   ?:                Ternary conditional                                       Right-to-left 
//
//              =                 Direct assignment (provided by default for C++ classes)
//              +=   −=           Assignment by sum and difference
//              *=   /=   %=      Assignment by product, quotient, and remainder
//              <<=   >>=         Assignment by bitwise left shift and right shift
//              &=   ^=   |=      Assignment by bitwise AND, XOR, and OR
//
//  17 OPP(0)   ,                 Comma                                                     Left-to-right 
//
// UVS 中不用的操作符号
//  1           ::                Scope resolution                                          Left-to-right
//  2           −>                Element selection through pointer                         Left-to-right
//  3           sizeof            Size-of                                                   Right-to-left
//  3           *                 Indirection (dereference)
//              new, new[]        Dynamic memory allocation
//              delete, delete[]  Dynamic memory deallocation
//              &                 Address-of
//
//  4           .*   ->*          Pointer to member                                         Left-to-right 
//  16          throw             Throw operator (for exceptions)                           Right-to-left 


//
// [GLSL 与 HLSL 语法差别]
// 1.GLSL支持“*”符号计算向量与矩阵，这个在uvs中也支持。
// 2.GLSL支持结构体简单构造：
//   struct Ray { vec3 pos, dir; };
//   Ray(vec3(0,0,0), vec3(1,1,1));
// uvs不支持这个。
// 3.GLSL支持“^^”符号，代表布尔异或，uvs不支持，查了一下，HLSL也没有这个符号。
// 4.GLSL支持数组的length()方法，uvs支持这个方法
// 5.GLSL支持数组初始化：
//  vec4 Scene[] = vec4[](vec4(0, 0, 0, 1), vec4(1, 1, 1, 1), vec4(2, 2, 2, 1));
// uvs不打算支持这种形式。

#define FOR_EACH_MBO(_N, _IDX) for(int _IDX = 0; s_Operator##_N[_IDX].szOperator != NULL; _IDX++)

#if defined(UVS_EXPORT_TEXT_IS_SIGN)
GXLPCSTR g_ExportErrorMessage2 = __FILE__;
#endif

namespace UVShader
{
  template void RecursiveNode<SYNTAXNODE>(ArithmeticExpression* pParser, SYNTAXNODE* pNode, std::function<GXBOOL(SYNTAXNODE*, int)> func, int depth);
  template void RecursiveNode<const SYNTAXNODE>(ArithmeticExpression* pParser, const SYNTAXNODE* pNode, std::function<GXBOOL(const SYNTAXNODE*, int)> func, int depth);
  static b32 s_bDumpScope = TRUE;
#if 0
  // 操作符号重载
  VALUE::State operator|(VALUE::State a, VALUE::State b)
  {
    return VALUE::State((u32)a | (u32)b);
  }

  void operator|=(VALUE::State& a, VALUE::State b)
  {
    a = VALUE::State((u32)a | (u32)b);
  }
#endif

#ifdef USE_CLSTD_TOKENS
  u32 ArithmeticExpression::m_aCharSem[128];
#endif // USE_CLSTD_TOKENS

  ArithmeticExpression::ArithmeticExpression()
    : m_pMsg(NULL)
    //, m_nMaxPrecedence(0)
    , m_nErrorCount(0)
    , m_nSessionError(0)
    , m_nDbgNumOfExpressionParse(0)
    , m_NodePool(128)
    , m_bHigherDefiniton(FALSE)
    , m_bRefMsg(FALSE)
#ifdef ENABLE_SYNTAX_NODE_ID
    , m_nNodeId(1)
#endif
  {
#ifdef _DEBUG
    // 检查名字与其设定长度是一致的
    //for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i) {
    //  ASSERT(GXSTRLEN(s_aIntrinsicType[i].name) == s_aIntrinsicType[i].name_len);
    //}
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
  }

  ArithmeticExpression::~ArithmeticExpression()
  {
    m_NodePool.Clear();
  }

  const TOKEN::Array* ArithmeticExpression::GetTokensArray() const
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
    if(remain < op_len) {
      return NULL;
    }

    for(int i = 0; op[i].szOperator != NULL; ++i) {
      ASSERT(op_len == op[i].nLen);
      if(clstd::strncmpT(op[i].szOperator, it.marker, op[i].nLen) == 0)
      {
        it->length = it.length = op[i].nLen;
        return &op[i];
      }
    }
    return NULL;
  }

  // 按照C语言的数学格式扩展token iterator
  // iterator在输入时可能是一个单纯的纯数字或者字符的串, 因为
  // 这个方法只是试图连接'.'或者E后面续接'+'/'-'而断开的两端字符串
  // 这个方法不会考虑'+','-'前缀, 带有这两个符号前缀会时扩展提前终止
  // 返回值: 如果做了扩展返回TRUE, 没有修改串长度返回FALSE, 该方法不会减少串长度
  b32 ArithmeticExpression::TryExtendNumeric(iterator &it, clsize remain)
  {
    b32 bENotation = FALSE;
    if((remain -= it.length) == 0) {
      return FALSE;
    }

    VALUE value;

    const int ec = *(it.end());
    const int front_is_digit = clstd::isdigit(it.front());
    const b32 exp_postfix = (it.back() == 'e' || it.back() == 'E');
    if((it.front() == '.' && clstd::isdigit(ec)) ||               // '.'+"数字..."
      (front_is_digit && exp_postfix) || // "数字...E/e"
      (front_is_digit && ec == '.'))                   // "数字..."+'.'
    {
      if(_CL_NOT_(front_is_digit && exp_postfix) || ec == '-' || ec == '+') {
        it.length++;
      }

      while(remain--)
      {
        if(isdigit(it.marker[it.length])) {
          it.length++;
        }
        else if(_CL_NOT_(bENotation) && // 没有切换到科学计数法时遇到‘e’标记
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
      return TRUE;
    }
    return FALSE;
  }

  u32 ArithmeticExpression::StepIterator(ArithmeticExpression::iterator& it)
  {
    size_t remain = m_pEnd - it.marker;
    if(it.BeginsWith('\"') && it.EndsWith('\"')) {
      //TOKEN& l_token = *(TOKEN*)lParam;
      it->type = TOKEN::TokenType_String;
      return 0;
    }

    //GXBOOL bIsLikeNumeric =  || 
    //  (it.length > 0 && isdigit(it.marker[0]));

    // 并不十分精确, 具体看应用时的解析
    if(TryExtendNumeric(it, remain)) {
      it->type = TOKEN::TokenType_Real;
    }
    else if(it.length > 0 && clstd::isdigit(it.marker[0])) {
      it->type = TOKEN::TokenType_Integer;
    }

    ASSERT((int)remain >= 0);
    return 0;
  }

  u32 ArithmeticExpression::MultiByteOperatorProc(iterator& it, u32 remain)
  {
    //ASSERT(lParam != NULL); // 需要 lParam 指向一个 TOKEN 结构体作为临时储存对象

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
      if(SmartStreamUtility::ExtendToCStyleBlockComment(it, 2, remain) == FALSE)
      {
        ArithmeticExpression* pThis = (ArithmeticExpression*)it.pContainer;
        pThis->m_pMsg->WriteErrorW(TRUE, it.offset(), UVS_EXPORT_TEXT2(1071, "在注释中遇到意外的文件结束", pThis));
      }
      ++it;
    }
    else
    {
      //TOKEN& l_token = *(TOKEN*)lParam;
      //CodeParser* pParser = (CodeParser*)it.pContainer;
      //ASSERT(it.marker == NULL); // 每次用完外面都要清理这个
      //ASSERT(it.marker == it.marker);
      //ASSERT(it.length == it.length);

      const MBO* pProp = NULL;
      // 从多字节到单字节符号匹配,其中有一个返回TRUE就不执行后面的匹配了
      if(
        (pProp = MatchOperator(s_Operator3, 3, it, remain)) ||
        (pProp = MatchOperator(s_Operator2, 2, it, remain)) ||
        (pProp = MatchOperator(s_Operator1, 1, it, remain)) )
      {
        //it->Set(it);
        it->precedence = pProp->precedence;
        it->scope = -1;
        it->unary = pProp->unary;
        it->unary_mask = pProp->unary_mask;
        it->type = TOKEN::TokenType_Operator;
      }
    }
    return 0;
  }

  ArithmeticExpression::iterator ArithmeticExpression::begin()
  {
    ArithmeticExpression::iterator it;

    it.marker = m_pBegin;
    it.length = 0;
    it.pContainer = this;
    it.operator++();

//    CTokens::iterator it_base = CTokens::begin();
//    it.marker     = it_base.marker;
//    it.length     = it_base.length;
//    it.pContainer = it_base.pContainer;
//    //it->Set(it_base);
//
//#ifdef ENABLE_STRINGED_SYMBOL
//    it_base.ToString(it.tk.symbol);
//#endif // #ifdef ENABLE_STRINGED_SYMBOL
//    it.tk.scope = -1;
//    it.tk.semi_scope = -1;
//    it.tk.pContainer = it_base.pContainer;
//    
//    it.tk.bPhony = 0;
//    it.tk.marker = it_base.marker;
//    it.tk.length = it_base.length;
//
//    StepIterator(it);
    return it;
  }

  ArithmeticExpression::iterator ArithmeticExpression::end()
  {
    CTokens::iterator it_base = CTokens::end();
    ArithmeticExpression::iterator it;
    it.marker = it_base.marker;
    it.length = it_base.length;
    it.pContainer = it_base.pContainer;
    //it->Set(it_base);
    return it;
  }

  const SYNTAXNODE* ArithmeticExpression::TryGetNode(const SYNTAXNODE::GLOB* pDesc) const
  {
    if(pDesc->IsNode()) {
      return pDesc->pNode;
    }
    return NULL;
  }

  SYNTAXNODE::MODE ArithmeticExpression::TryGetNodeMode(const SYNTAXNODE::GLOB* pDesc) const
  {
    if(pDesc->IsNode()) {
      return pDesc->pNode->mode;
    }
    return SYNTAXNODE::MODE_Undefined;
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::GLOB* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::GLOB* pOperandA, SYNTAXNODE::GLOB* pOperandB)
  {
    return MakeSyntaxNode(pDest, mode, NULL, pOperandA, pOperandB);
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::GLOB* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::GLOB* pOperandA, SYNTAXNODE::GLOB* pOperandB)
  {
    //const SYNTAXNODE::GLOB* pOperand[] = {pOperandA, pOperandB};
    SYNTAXNODE sNode; // = {SYNTAXNODE::FLAG_OPERAND_MAGIC, mode, pOpcode};
    sNode.magic   = SYNTAXNODE::FLAG_OPERAND_MAGIC;
    sNode.mode    = mode;
    sNode.pOpcode = pOpcode;
    sNode.Operand[0].ptr = (pOperandA == NULL) ? NULL : pOperandA->ptr;
    sNode.Operand[1].ptr = (pOperandB == NULL) ? NULL : pOperandB->ptr;

    pDest->pNode = m_NodePool.PushBack(sNode);

#ifdef ENABLE_SYNTAX_NODE_ID
    pDest->pNode->id = m_nNodeId++;
#endif

    return TRUE;
  }

  GXBOOL ArithmeticExpression::MakeInstruction(int depth, const TOKEN* pOpcode, int nMinPrecedence, const TKSCOPE* pScope, SYNTAXNODE::GLOB* pParent, int nMiddle)
  {
    ASSERT((int)pScope->begin <= nMiddle);
    ASSERT(nMiddle <= (int)pScope->end);

    TKSCOPE scopeA(pScope->begin, nMiddle);
    TKSCOPE scopeB(nMiddle + 1, pScope->end);
    SYNTAXNODE::GLOB A = {0}, B = {0};
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

    // 解析中保证一元操作符解析正确
    if(bresult && pOpcode->unary) {
      if(A.pNode != NULL && B.pNode != NULL)
      {
        // ERROR: 一元操作符不能同时带有左右操作数
        return FALSE;
      }
      else if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_LEFT_OPERAND) && A.pNode != NULL)
      {
        // ERROR: 一元操作符不接受左值
        return FALSE;
      }
      else if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_RIGHT_OPERAND) && B.pNode != NULL)
      {
        // ERROR: 一元操作符不接受右值
        return FALSE;
      }
    }

    return bresult;
  }

  GXBOOL ArithmeticExpression::IsLikeTypeCast(const TKSCOPE& scope, TKSCOPE::TYPE i)
  {
    const TOKEN& front = m_aTokens[scope.begin];
    if(front == '(' && (TKSCOPE::TYPE)front.scope + 1 < scope.end) // (...)... 形式
    {
      const TOKEN& nt = m_aTokens[front.scope + 1];
      if((nt == '(' && nt.scope == scope.end - 1) ||
        nt.unary || nt.IsIdentifier() || nt.IsNumeric()) // type cast
      {
        return TRUE;
      }
    }
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////

  ArithmeticExpression::TKSCOPE::TYPE ArithmeticExpression::GetLowestPrecedence(const TKSCOPE& scope, int nMinPrecedence)
  {
    // 获得一个区间的操作符最低优先级的索引

    int nCandidate = s_MaxPrecedence;
    TKSCOPE::TYPE i = scope.end - 1;
    TKSCOPE::TYPE nCandidatePos = i;

    while(nMinPrecedence <= s_MaxPrecedence)
    {
      if(nMinPrecedence == OPP(1) || nMinPrecedence == OPP(12))
      {
        for(i = scope.begin; i < scope.end; ++i)
        {
          m_nDbgNumOfExpressionParse++;

          const TOKEN& s = m_aTokens[i];

          if(s.precedence == TOKEN::ID_BRACE) // 跳过非运算符, 也包括括号
          {
            ASSERT(s.scope < (int)scope.end); // 闭括号肯定在表达式区间内
            if(IsLikeTypeCast(scope, i)) {
              return s.scope;
            }
            i = s.scope;
            continue;
          }
          else if(s.precedence == 0 || s == ':') { // 跳过非运算符, 这里包括三元运算符的次级运算符
            continue;
          }

          // ?: 操作符标记：precedence 储存优先级，scope 储存?:的关系

          if(s.precedence == nMinPrecedence) {
            return i;
            //return MakeInstruction(depth + 1, &s, nMinPrecedence, &scope, pDesc, i);
          }
          else if(s.precedence < nCandidate) {
            nCandidate = s.precedence;
            // 这里优先级因为从LTR切换到RTL，所以不记录 nCandidatePos
          }
        } // for

        nCandidatePos = scope.end - 1;
      }
      else
      {
        for(; (GXINT_PTR)i >= (GXINT_PTR)scope.begin; --i)
        {
          m_nDbgNumOfExpressionParse++;
          const TOKEN& s = m_aTokens[i];

          // 优先级（2）是从右向左的，这个循环处理从左向右
          ASSERT(nMinPrecedence != 2);

          // 跳过非运算符, 也包括括号
          if(s.precedence == TOKEN::ID_BRACE)
          {
            if(s.scope >= (int)scope.end) // 闭括号肯定在表达式区间内
            {              
              return TKSCOPE::npos;
            }
#if 1
            if(IsLikeTypeCast(scope, i)) {
              return i;
            }
#endif
            i = s.scope;
            continue;
          }
          else if(s.precedence == 0) { // 跳过非运算符
            continue;
          }

          if(s.precedence == nMinPrecedence) {
            return i;
            //return MakeInstruction(depth + 1, &s, nMinPrecedence, &scope, pDesc, i);
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
    return TKSCOPE::npos;
  }

  void ArithmeticExpression::EnableHigherDefinition(GXBOOL bHigher)
  {
    m_bHigherDefiniton = bHigher;
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    return ParseArithmeticExpression(depth + 1, scope, pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, int nMinPrecedence)
  {
    SYNTAXNODE::GLOB A, B;

    if(depth > 1000)
    {
      // ERROR: 表达式解析堆栈不足
      m_pMsg->WriteErrorW(TRUE, m_aTokens[scope.begin].offset(), UVS_EXPORT_TEXT(5001, "表达式解析堆栈不足"));
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

      ASSERT(*B.pTokn != ';'); // 已经在外部避免了表达式内出现分号

      if(A.pTokn->precedence > 0)
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, A.pTokn, NULL, &B);
        DbgDumpScope(A.pTokn->ToString(), TKSCOPE(0,0), TKSCOPE(scope.begin + 1, scope.end));
      }
      else if(B.pTokn->precedence > 0)
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, B.pTokn, &A, NULL);
        DbgDumpScope(B.pTokn->ToString(), TKSCOPE(scope.begin, scope.begin + 1), TKSCOPE(0,0));
      }
      else {
        // 变量声明
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Definition, &A, &B);
      }
      return bret;
    }
    else if(front.precedence == 0 && m_aTokens[scope.begin + 1].precedence == 0 && m_bHigherDefiniton == FALSE) // 变量声明
    {
      ASSERT(count > 2);
      SYNTAXNODE::MODE mode = SYNTAXNODE::MODE_Definition;
      TKSCOPE scope_expr(scope.begin + 1, scope.end);
      A = front;
      B.ptr = NULL;
      GXBOOL bret = ParseArithmeticExpression(depth + 1, scope_expr, &B);
      bret = bret && MakeSyntaxNode(pDesc, mode, &A, &B);
      return bret;
    }
    else if((front == '(' || front == '[' || front == '{') && front.scope == scope.end - 1)  // 括号内表达式
    {
      // (...) 形式
      
      ASSERT(m_aTokens[scope.end - 1].scope == scope.begin); // 括号肯定是匹配的
      return ParseArithmeticExpression(depth + 1, TKSCOPE(scope.begin + 1, scope.end - 1), pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
    }
    else if(front.IsIdentifier() && m_aTokens[scope.begin + 1].scope == scope.end - 1)  // 整个表达式是函数调用
    {
      // X(...) 形式
      return ParseFunctionCall(scope, pDesc);
    }

    //
    // 获得结合律最低的符号位置
    //
    TKSCOPE::TYPE nLowestOpcodeIndex = GetLowestPrecedence(scope, nMinPrecedence);
    ASSERT(nLowestOpcodeIndex == TKSCOPE::npos ||
      (scope.begin <= nLowestOpcodeIndex && nLowestOpcodeIndex < scope.end));
    //
    //

    if(nLowestOpcodeIndex != TKSCOPE::npos)
    {
      const TOKEN& t = m_aTokens[nLowestOpcodeIndex];
      ASSERT(t == ')' || (OPP(0) <= t.precedence && t.precedence <= s_MaxPrecedence));

      if(t == ')')
      {
        ASSERT(m_aTokens[scope.begin] == '(');
        return ParseTypeCast(scope, pDesc);
      }

      return MakeInstruction(depth + 1, &t,
        t.precedence, &scope, pDesc, nLowestOpcodeIndex);
    }
#if 1
    return ParseFunctionIndexCall(scope, pDesc);
#else
    if( ! ParseFunctionIndexCall(scope, pDesc))
    {
      GXINT_PTR len = (m_aTokens[scope.end - 1].marker - m_aTokens[scope.begin].marker) + m_aTokens[scope.end - 1].length;
      ASSERT(len >= 0);
      clStringA strMsg(m_aTokens[scope.begin].marker, len);
      clStringW strMsgW = strMsg;
      m_pMsg->WriteErrorW(TRUE, m_aTokens[scope.begin].offset(), UVS_EXPORT_TEXT(5008, "无法解析表达式: \"%s\"."), strMsgW.CStr());
      return FALSE;
    }
    return TRUE;
#endif
  }

  GXBOOL ArithmeticExpression::DbgHasError(int errcode) const
  {
    return (m_errorlist.find(errcode) != m_errorlist.end());
  }

  size_t ArithmeticExpression::DbgErrorCount() const
  {
    return m_nErrorCount;
  }

  GXBOOL ArithmeticExpression::ParseFunctionIndexCall(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    // 从右到左解析这两种形式:
    // name(...)(...)(...)
    // name[...][...][...]
    // 括号域之间不能有其他符号, 括号域之内是数学表达式

    struct CONTEXT
    {
      SYNTAXNODE::MODE mode;
      SYNTAXNODE::GLOB B;
    };

    typedef clstack<CONTEXT> SyntaxStack;
    SyntaxStack node_stack;
    SYNTAXNODE::GLOB A;
    CONTEXT c;
    TOKEN* pBack = &m_aTokens[scope.end - 1];
    A = m_aTokens[scope.begin];
    //ASSERT(A.pTokn->precedence == 0); // 第一个必须不是运算符号
    if(A.pTokn->precedence != 0)
    {
      clStringW strW;
      m_pMsg->WriteErrorW(TRUE, A.pTokn->offset(), UVS_EXPORT_TEXT(5040, "语法错误: “%s”"), A.pTokn->ToString(strW).CStr());
      return FALSE;
    }


    while(1) {
      if(pBack->scope == TKSCOPE::npos) {
        ERROR_MSG__MISSING_SEMICOLON(*A.pTokn);
        return FALSE;
      }

      if(*pBack == ')') {
        c.mode = SYNTAXNODE::MODE_FunctionCall;
      }
      else if(*pBack == ']')
      {
        c.mode = SYNTAXNODE::MODE_Subscript;
      }
      else {
        ASSERT(*pBack == '}');
        clStringW str;
        m_pMsg->WriteErrorW(TRUE, A.pTokn->offset(), UVS_EXPORT_TEXT(2054, "在“%s”之后应输入“%s”"), A.pTokn->ToString(str).CStr(), _CLTEXT("="));
        c.mode = SYNTAXNODE::MODE_Undefined;
      }

      c.B.ptr = NULL;

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

#if defined(UVS_EXPORT_TEXT_IS_SIGN)
  GXUINT ArithmeticExpression::MarkCode(GXUINT code, GXLPCSTR szMessage)
  {
    clStringW strMessage = szMessage;
    m_pMsg->UpdateErrorMessage(code, strMessage);
    return code;
  }
#endif

  GXBOOL ArithmeticExpression::ParseFunctionCall(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    // 括号肯定是匹配的
    ASSERT(m_aTokens[scope.end - 1].scope == scope.begin + 1);

    SYNTAXNODE::GLOB A, B = {0};
    A = m_aTokens[scope.begin];

#if 0 // 外部保证
    // 检查m_aTokens[scope.begin]是函数名, 或者正号, 负号
    if(*A.pTokn != '+' && *A.pTokn != '-' && *A.pTokn != '!' &&
      A.pTokn->IsIdentifier() == FALSE) // 检查是否为标识符
    {
      clStringW str;
      m_pMsg->WriteErrorW(TRUE, A.pTokn->offset(), UVS_EXPORT_TEXT(5005, "表达式看起来像函数, 但是\"%s\"不是标识符."), A.pTokn->ToString(str).CStr());
      return FALSE;
    }
#endif
    // TODO: 重名/重载检查?

    const TOKEN& bracket = m_aTokens[scope.begin + 1];
    ASSERT(bracket == '[' || bracket == '(');
    GXBOOL bret = ParseArithmeticExpression(0, TKSCOPE(scope.begin + 2, scope.end - 1), &B, TOKEN::FIRST_OPCODE_PRECEDENCE);

    SYNTAXNODE::MODE mode;
    if(bracket == '(') {
      mode = SYNTAXNODE::MODE_FunctionCall;
    }
    else if(bracket.scope == m_aTokens[bracket.scope].scope + 1) // "[]"
    {
      mode = SYNTAXNODE::MODE_Subscript0;
    }
    else {
      mode = SYNTAXNODE::MODE_Subscript;
    }

    MakeSyntaxNode(pDesc, mode, &A, &B);
    DbgDumpScope(bracket == '(' ? "F" : "I", TKSCOPE(scope.begin, scope.begin + 1),
      TKSCOPE(scope.begin + 2, scope.end - 1));

    return bret;
  }

  GXBOOL ArithmeticExpression::ParseTypeCast(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    ASSERT(scope.begin < scope.end - 2); // 这个由之前的判断保证, 目前括号里只可能有一个类型标识符
    //ASSERT(m_aTokens[scope.begin].scope == scope.end - 2); // 外部保证是(A)B 形式

    SYNTAXNODE::GLOB A = {0}, B = {0};
    //B = m_aTokens[scope.end - 1];

    //if(B.pTokn->IsIdentifier() == FALSE)
    //{
    //  clStringW str;
    //  m_pMsg->WriteErrorW(TRUE, B.pTokn->offset(), UVS_EXPORT_TEXT(5006, "表达式看起来像类型转换, 但是\"%s\"不是标识符."), B.pTokn->ToString(str).CStr());
    //  return FALSE;
    //}

    const TOKEN& bracket = m_aTokens[scope.begin];
    ASSERT(bracket == '(');

    TKSCOPE type_scope; //(scope.begin + 1, m_aTokens[scope.begin].scope);
    TKSCOPE cast_scope;

    InitTokenScope(type_scope, scope.begin, FALSE);
    cast_scope.begin = type_scope.end + 1;
    cast_scope.end = scope.end;
    ASSERT(cast_scope.begin < cast_scope.end);

    if(_CL_NOT_(ParseArithmeticExpression(0, type_scope, &A, TOKEN::FIRST_OPCODE_PRECEDENCE)))
    {
      m_pMsg->WriteErrorW(TRUE, m_aTokens[type_scope.begin].offset(), UVS_EXPORT_TEXT(5007, "类型转换:类型无法解析."));
      return FALSE;
    }

    if(_CL_NOT_(ParseArithmeticExpression(0, cast_scope, &B, OPP(12))))
    {
      //m_pMsg->WriteErrorW(TRUE, m_aTokens[type_scope.begin].offset(), UVS_EXPORT_TEXT(5008, "类型转换:表达式无法解析."));
      return FALSE;
    }

    MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_TypeCast, &A, &B);

    DbgDumpScope("C", type_scope, cast_scope);

    return TRUE;
  }

  void ArithmeticExpression::InitTokenScope(TKSCOPE& scope, const TOKEN& token, b32 bHasBracket) const
  {
    // 使用token的括号作用域初始化一个作用域
    ASSERT(&token >= &m_aTokens.front() && &token < &m_aTokens.back()); // token必须是本类的序列中的
    ASSERT(token.scope != -1); // 必须有匹配

    if(bHasBracket)
    {
      scope.end = token.scope + 1;
      scope.begin = m_aTokens[token.scope].scope;
    }
    else
    {
      scope.end = token.scope;
      scope.begin = m_aTokens[token.scope].scope + 1;
    }
  }

  void ArithmeticExpression::InitTokenScope(TKSCOPE& scope, GXUINT index, b32 bHasBracket) const
  {
    // 使用token的括号作用域初始化一个作用域
    ASSERT(index < (GXUINT)m_aTokens.size()); // token必须是本类的序列中的
    ASSERT(m_aTokens[index].scope != -1); // 必须有匹配

    if(bHasBracket)
    {
      scope.begin = index;
      scope.end   = m_aTokens[index].scope + 1;
    }
    else
    {
      scope.begin = index + 1;
      scope.end = m_aTokens[index].scope;
    }
  }

  GXBOOL ArithmeticExpression::MarryBracket(PairStack* sStack, TOKEN& token, GXBOOL bSilent)
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
          }
          else if(_CL_NOT_(bSilent)){
            // ERROR: 括号不匹配
            clStringW str((clStringW::TChar)c.chOpen, 1);
            m_pMsg->WriteErrorW(TRUE, token.offset(), UVS_EXPORT_TEXT(2059, "括号不匹配, 缺少\"%s\"."), str.CStr());
          }
          break;
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

      return FALSE;
        //EOE = c_size + 1;
    } // for
    return FALSE;
  }

  GXBOOL ArithmeticExpression::IsArrayList(const TOKEN& token)
  {
    if(token != '}' || m_aTokens.empty()) {
      return FALSE;
    }

    // 数组列表
    // 1. ={}
    // 2. {没有分号}

    if(m_aTokens.back() == '{') {
      return CompareToken(token.scope - 1, "=");
    }
    return (m_aTokens[token.scope + 1].semi_scope == -1);
  }

  //////////////////////////////////////////////////////////////////////////

  void ArithmeticExpression::DbgDumpScope( clStringA& str, clsize begin, clsize end, GXBOOL bRaw )
  {
    if(_CL_NOT_(s_bDumpScope)) {
      return;
    }

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
    if(_CL_NOT_(s_bDumpScope)) {
      return;
    }

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

  void ArithmeticExpression::Invoke(GXLPCSTR szFunc, GXLPCSTR szArguments)
  {
    if(clstd::strcmpT(szFunc, "EnableDumpScope"))
    {
      s_bDumpScope = clStringA(szArguments).ToBoolean();
    }
  }

  const clStringArrayA& ArithmeticExpression::DbgGetExpressionStack() const
  {
    return m_aDbgExpressionOperStack;
  }

  int ArithmeticExpression::SetError(int err)
  {
    m_nErrorCount++;
    m_nSessionError++;
    m_errorlist.insert(err);

    if(m_nErrorCount == c_nMaxErrorCount) {
      m_pMsg->WriteErrorW(FALSE, 0, UVS_EXPORT_TEXT(9997, "错误数量超过%d条，将停止输出错误消息"), c_nMaxErrorCount);
    }
    return err;
  }

  void ArithmeticExpression::ResetSessionError()
  {
    m_nSessionError = 0;
  }

  ArithmeticExpression::TChar ArithmeticExpression::GetPairOfBracket(TChar ch)
  {
    for(int i = 0; i < countof(s_PairMark); i++)
    {
      if(s_PairMark[i].bCloseAE == FALSE) {
        if(s_PairMark[i].chOpen == ch) {
          return s_PairMark[i].chClosed;
        }
        else if(s_PairMark[i].chClosed == ch) {
          return s_PairMark[i].chOpen;
        }
      }
    }
    CLBREAK; // 不是括号!
    return '?';
  }

  //////////////////////////////////////////////////////////////////////////

  void VALUE::clear()
  {
    uValue64 = 0;
    rank = Rank_Undefined;
  }

  void VALUE::SetZero()
  {
    uValue64 = 0;
    rank = Rank_Signed;
  }

  void VALUE::SetOne()
  {
    uValue64 = 1;
    rank = Rank_Signed;
  }

  VALUE& VALUE::set(const VALUE& v)
  {
    *this = v;
    return *this;
  }

  VALUE::State VALUE::set(const TOKEN& token)
  {
    return set(token.marker, token.length, token.type == TOKEN::TokenType_Integer);
  }

  VALUE::State VALUE::set(TOKEN::T_LPCSTR ptr, size_t count, b32 bInteger)
  {
    // 注意，有关上限的检查写的不严谨(貌似已经严谨了)
    // X是任意内容，D是指数字
    // "-X" "+X" ".X" "Xf"
    // "De-D" "DeD"
    //auto ptr     = token.marker;
    //size_t count = token.length;
    GXDWORD dwFlags = 0;
    GXQWORD digi[3] = {0}; // [0]是整数部分，[1]是小数部分, [2]是指数
    size_t p = 0; // part
    size_t i = 0;
    GXBOOL bNegExp = FALSE;
    ASSERT(count != 0); // 不可能为空

    rank = Rank_BadValue;

    // FIXME: 这种写法会认为"10f"为合法的,实际C/C++中不承认这种写法
    if(ptr[count - 1] == 'f' || ptr[count - 1] == 'F') {
      SET_FLAG(dwFlags, Rank_Float);
      count--;
    }

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

    if(ptr[i] == '0' && bInteger) // 8进制
    {
      for(i++; i < count; i++)
      {
        if(ptr[i] == '8' || ptr[i] == '9') {
          return State_IllegalNumber;
        }
        else if(ptr[i] >= '0' && ptr[i] <= '7')
        {
          const int n = ptr[i] - '0';

          if(ptr[i] > (ULLONG_MAX / 8) || (ptr[i] == (ULLONG_MAX / 8) && n > (ULLONG_MAX % 8)))
          {
            return State_Overflow;
          }
          digi[p] = digi[p] * 8 + n;
        }
        else {
          return State_IllegalChar;
        }
      }
    }
    else
    {
      for(; i < count; i++)
      {
        if(ptr[i] == '.')
        {
          SET_FLAG(dwFlags, Rank_Float);
          p++;
          if(p >= 2) { // 不会出现两个‘.’, 这个由TOKEN中的浮点数分析保证
            return State_SyntaxError;
          }
        }
        else if(ptr[i] == 'e' || ptr[i] == 'E')
        {
          p = 2;
          i++;
          SET_FLAG(dwFlags, Rank_Float);
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
          if(dwFlags != Rank_Float) {
            ASSERT(p == 0);
            if(digi[p] > (ULLONG_MAX / 10) || (digi[p] == (ULLONG_MAX / 10) && n > (ULLONG_MAX % 10))) {
              return State_Overflow;
            }
          }
          digi[p] = digi[p] * 10 + n;
        }
        else if(p == 0 && (ptr[i] == 'u' || ptr[i] == 'U') && i == count - 1)
        {
          dwFlags = Rank_Unsigned;
          break;
        }
        else {
          return State_IllegalChar;
        }
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

    if(dwFlags == Rank_Float)
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

      if(TEST_FLAG(dwFlags, Rank_Signed))
      {
        if(nValue64 > 0x8000000000000000) {
          return State_Overflow;
        }
        else if(nValue64 > 0x80000000) {
          SET_FLAG(dwFlags, Rank_F_LongLong);
        }
        nValue64 = -nValue64;
      }
      else
      {
        if(nValue64 > 0xffffffff)
        {
          SET_FLAG(dwFlags, Rank_F_LongLong);
        }
      }

      //if((TEST_FLAG(dwFlags, Rank_Signed) && uValue64 > INT_MAX) ||
      //  (TEST_FLAG_NOT(dwFlags, Rank_Signed) && uValue64 > UINT_MAX)) {
      //    SETBIT(dwFlags, Rank_F_LongLong);
      //}
    }

    rank = (Rank)dwFlags;
    return State_OK;
  }

  template<typename _Ty>
  VALUE::State VALUE::CalculateT(_Ty& output, const TOKEN& opcode, const _Ty& t1, const _Ty& t2)
  {
    if(opcode.length == 1)
    {
      switch(opcode.marker[0])
      {
      case '+': output = t1 + t2; break;
      case '-': output = t1 - t2; break;
      case '*': output = t1 * t2; break;
      case '/': output = t1 / t2; break;
      case '<': output = _Ty(t1 < t2); break;
      case '>': output = _Ty(t1 > t2); break;
      case '!': output = _Ty( ! t2); break;
      default:
        return State_UnknownOpcode;
        //TRACE("Unsupport opcode(%c).\n", opcode);
        //CLBREAK;
      }
    }
    else
    {
      if(opcode == "&&") {
        output = (t1 && t2);
      }
      else if(opcode == "||") {
        output = (t1 || t2);
      }
      else if(opcode == "==") {
        output = (t1 == t2);
      }
      else {
        return State_UnknownOpcode;
        //TRACE("Unsupport opcode(%c).\n", opcode);
        //CLBREAK;
      }
    }

    return State_OK;
  }

  VALUE::State VALUE::Calculate(const TOKEN& token, const VALUE& param0, const VALUE& param1)
  {
    *this = param0;
    VALUE second = param1;
    Rank type = clMax(this->rank, second.rank);
    State state = UpgradeValueByRank(type);
    state = (state == State_OK) ? second.UpgradeValueByRank(type) : state;
    
    if(state != State_OK) {
      return state;
    }

    if(type == Rank_Signed64) {
      state = CalculateT(nValue64, token, nValue64, second.nValue64);
    }
    else if(type == Rank_Unsigned64) {
      state = CalculateT(uValue64, token, uValue64, second.uValue64);
    }
    else if(type == Rank_Double) {
      state = CalculateT(fValue64, token, fValue64, second.fValue64);
    }
    else if(type == Rank_Unsigned) {
      state = CalculateT(uValue, token, uValue, second.uValue);
    }
    else if(type == Rank_Signed) {
      state = CalculateT(nValue, token, nValue, second.nValue);
    }
    else {
      CLBREAK;
      return State_SyntaxError;
    }
    rank = type;
    return state;
  }

  clStringA VALUE::ToString() const
  {
    clStringA str;
    switch(rank)
    {
    case Rank_Signed:
      str.AppendInteger32(nValue);
      break;
    case Rank_Unsigned:
      str.AppendUInt32(uValue);
      break;
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

  VALUE::State VALUE::UpgradeValueByRank(Rank _type)
  {
    if(rank == _type || rank == Rank_Undefined) { // 同级或者未定义（一元操作情况）
      return State_OK;
    }
    else if(rank == Rank_BadValue || rank > _type) {
      return State_SyntaxError;
    }

    if(_type == Rank_Float) {
      float r;
      ASSERT(rank == Rank_Unsigned || rank == Rank_Signed); // 只可能是这两种情况
      if(TEST_FLAG(rank, Rank_Signed)) {
        r = (float)nValue64;
      } else {
        r = (float)uValue64;
      }
      fValue = r;
    }
    else if(_type == Rank_Double) {
      double d;
      if(rank == Rank_Float) {
        d = (double)fValue;
      } else if(TEST_FLAG(rank, Rank_Signed)) {
        d = (double)nValue64;
      } else {
        d = (double)uValue64;
      }
      fValue64 = d;
    }
    else if(_type == Rank_Signed) {
      ASSERT(rank == Rank_Unsigned);
      if(nValue & 0x80000000) {
        return State_Overflow;
      }
    }
    else if(_type == Rank_Signed64 && rank == Rank_Signed) {
      // 高位肯定初始化了
      ASSERT((nValue >= 0 && (nValue64 & 0xffffffff00000000) == 0) ||
        (nValue < 0 && (nValue64 & 0xffffffff00000000) == 0xffffffff00000000)
      );
    }
    else if(_type == Rank_Signed64 && rank == Rank_Unsigned) {
      ASSERT((uValue64 & 0xffffffff00000000) == 0); // 高位肯定初始化为0了
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

  VALUE::State VALUE::CastValueByRank(Rank new_rank)
  {
    if(rank == new_rank) {
      return State_OK;
    }
    const Rank old_rank = rank;
    rank = new_rank;

    const u64 u32_max = 0xffffffff;

    switch(new_rank)
    {
    case UVShader::VALUE::Rank_Unsigned:
    {
      switch(old_rank)
      {
      case UVShader::VALUE::Rank_Signed:
      {
        // const u32 v = -1; // 4294967295U
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        return State_OK;
      }

      case UVShader::VALUE::Rank_Float:
      {
        //const u32 v = 1.23f; // 可能丢失数据
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        uValue = static_cast<u32>(fValue);
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Unsigned64:
      case UVShader::VALUE::Rank_Signed64:
      {
        //const u32 v = 0x100000000;
        //const u32 v = -1Ui64;
        uValue64 = static_cast<u64>(uValue);
        return State_Truncation;
      }
      case UVShader::VALUE::Rank_Double:
      {
        //const u32 v = (double)5000000000.0;
        uValue64 = static_cast<u64>(fValue64) & u32_max;
        return State_LoseOfData;
      }
      default:
        CLBREAK;
        break;
      }
      break;
    }
    case UVShader::VALUE::Rank_Signed:
    {
      switch(old_rank)
      {
      case UVShader::VALUE::Rank_Unsigned:
      {
        //const s32 v = 4294967295U;
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Float:
      {
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        nValue = static_cast<s32>(fValue);
        return State_LoseOfData;
        break;
      }
      case UVShader::VALUE::Rank_Unsigned64:
      case UVShader::VALUE::Rank_Signed64:
      {
        uValue64 = static_cast<u64>(uValue);
        return State_Truncation;
      }
      case UVShader::VALUE::Rank_Double:
      {
        nValue64 = static_cast<u64>(fValue64) & u32_max;
        return State_LoseOfData;
      }
      default:
        CLBREAK;
        break;
      }
      break;
    }
    case UVShader::VALUE::Rank_Float:
    {
      switch(old_rank)
      {
      case UVShader::VALUE::Rank_Unsigned:
      {
        fValue = static_cast<float>(uValue);
        uValue64 &= u32_max;
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Signed:
      {
        fValue = static_cast<float>(nValue);
        uValue64 &= u32_max;
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Unsigned64:
      {
        fValue = static_cast<float>(uValue64);
        uValue64 &= u32_max;
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Signed64:
      {
        fValue = static_cast<float>(nValue64);
        uValue64 &= u32_max;
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Double:
      {
        fValue = static_cast<float>(fValue64);
        uValue64 &= u32_max;
        return State_LoseOfData;
      }
      default:
        CLBREAK;
        break;
      }
      break;
    }
    case UVShader::VALUE::Rank_Unsigned64:
    {
      switch(old_rank)
      {
      case UVShader::VALUE::Rank_Unsigned:
      case UVShader::VALUE::Rank_Signed:
      {
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Float:
      {
        //const float vfrom = 1.2f;
        //const u64 v = vfrom;
        uValue64 = static_cast<u64>(fValue);
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Signed64:
      {
        return State_OK;
      }
      case UVShader::VALUE::Rank_Double:
      {
        //const double vfrom = 1.2;
        //const u64 v = vfrom;
        uValue64 = static_cast<u64>(fValue64);
        return State_LoseOfData;
      }
      default:
        CLBREAK;
        break;
      }
      break;
    }
    case UVShader::VALUE::Rank_Signed64:
    {
      switch(old_rank)
      {
      case UVShader::VALUE::Rank_Unsigned:
      {
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Signed:
      {
        //const s32 vfrom = -1;
        //const s64 v = vfrom;
        ASSERT((uValue64 & 0xffffffff00000000) == 0);
        nValue64 = static_cast<i64>(nValue); // 补码
        return State_OK;
      }
      case UVShader::VALUE::Rank_Float:
      {
        nValue64 = static_cast<s64>(fValue);
        return State_LoseOfData;
      }
      case UVShader::VALUE::Rank_Unsigned64:
      {
        return State_OK;
      }
      case UVShader::VALUE::Rank_Double:
      {
        nValue64 = static_cast<s64>(fValue64);
        return State_LoseOfData;
      }
      default:
        CLBREAK;
        break;
      }
      break;
    }
    case UVShader::VALUE::Rank_Double:
    {
      switch(old_rank)
      {
      case UVShader::VALUE::Rank_Unsigned:
      {
        fValue64 = static_cast<double>(uValue);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Signed:
      {
        fValue64 = static_cast<double>(nValue);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Float:
      {
        fValue64 = static_cast<double>(fValue);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Unsigned64:
      {
        fValue64 = static_cast<double>(uValue64);
        return State_OK;
      }
      case UVShader::VALUE::Rank_Signed64:
      {
        fValue64 = static_cast<double>(nValue64);
        return State_OK;
      }
      default:
        CLBREAK;
        break;
      }
      break;
    }
    default:
      CLBREAK;
      break;
    }
    return State_OK;
  }

  //VALUE::State VALUE::SyncLevel(VALUE& t1, VALUE& t2)
  //{
  //  Type type = clMax(t1.type, t2.type);
  //  
  //}
  
  //////////////////////////////////////////////////////////////////////////
  void TOKEN::ClearMarker()
  {
#ifdef ENABLE_STRINGED_SYMBOL
    symbol.Clear();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    marker = 0;
    length = 0;
    type = TokenType_Undefine;
    bPhony = 0;
  }

//  void TOKEN::Set(const iterator& _iter)
//  {
//    //ASSERT((size_t)this != (size_t)&_iter + offsetof(ArithmeticExpression::iterator, tk));
//    if(_iter.length != 0)
//    {
//#ifdef ENABLE_STRINGED_SYMBOL
//      symbol = _iter.ToString();
//#endif // #ifdef ENABLE_STRINGED_SYMBOL
//      scope = -1;
//      semi_scope = -1;
//      pContainer = _iter.pContainer;
//
//      bPhony = 0;
//    }
//    marker = _iter.marker;
//    length = _iter.length;
//  }

  void TOKEN::Set(clstd::StringSetA& sStrSet, const clStringA& str)
  {
    ASSERT(str.IsNotEmpty());
#ifdef ENABLE_STRINGED_SYMBOL
    symbol = str;
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    pContainer = NULL;
    marker     = sStrSet.add(str);
    length     = str.GetLength();
    bPhony     = 1;
  }

  void TOKEN::SetPhonyString(T_LPCSTR szText, size_t len)
  {
    ASSERT(szText != NULL && length > 0);
#ifdef ENABLE_STRINGED_SYMBOL
    symbol.Clear();
    symbol.Append(szText, length);
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    pContainer = NULL;
    marker     = szText;
    length     = len;
    bPhony     = 1;
  }

  void TOKEN::ClearArithOperatorInfo()
  {
    unary      = 0;
    unary_mask = 0;
    precedence = 0;
  }

  clStringA TOKEN::ToString() const
  {
    if(type == TokenType_String && ! BeginsWith('\"')) {
      return clStringA("\"") + ToRawString() + '\"';
    }
    return ToRawString();
  }

  clStringA& TOKEN::ToString(clStringA& str) const
  {
    str.Clear();
    if(type == TokenType_String && !BeginsWith('\"')) {
      return str.Append("\"").Append(marker, length).Append("\"");
    }
    return ToRawString(str);
  }

  clStringW& TOKEN::ToString(clStringW& str) const
  {
    clStringA strA;
    str = ToString(strA);
    return str;
  }

  int TOKEN::GetScope() const
  {
    return scope >= 0 ? scope : semi_scope;
  }

  GXBOOL TOKEN::operator==(const TOKEN& t) const
  {
    // 不可能出现指向同一地址却长度不同的情况
    ASSERT((marker == t.marker && length == t.length) || 
      (marker != t.marker));

    return (marker == t.marker) || (length == t.length
      && GXSTRNCMP(marker, t.marker, length) == 0);
  }

  GXBOOL TOKEN::operator==(SmartStreamA::T_LPCSTR str) const
  {
    return (*static_cast<const iterator*>(this) == str);
  }

  GXBOOL TOKEN::operator==(SmartStreamA::TChar ch) const
  {
    return (*static_cast<const iterator*>(this) == ch);
  }

  GXBOOL TOKEN::operator!=(SmartStreamA::T_LPCSTR str) const
  {
    return (*static_cast<const iterator*>(this) != str);
  }

  GXBOOL TOKEN::operator!=(SmartStreamA::TChar ch) const
  {
    return (*static_cast<const iterator*>(this) != ch);
  }

  b32 TOKEN::operator<(const TOKEN& _token) const
  {
    return (b32)(*static_cast<const iterator*>(this) < _token);
  }

  b32 TOKEN::IsIdentifier() const
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

  b32 TOKEN::IsNumeric() const
  {
    return TokenType_FirstNumeric < type && type < TokenType_LastNumeric;
  }

  clsize TOKEN::offset() const
  {
    if((size_t)pContainer & 0x1) {
      // 代换的token使用pContainer储存偏移
      return ((size_t)pContainer & (~(size_t)1));
    }
    return CTokens::iterator::offset();
  }

  //TOKEN& TOKEN::operator++()
  //{
  //  //CTokens::iterator::GetNext()
  //  pContainer->next(*this);

  //  //(reinterpret_cast<const ArithmeticExpression*>(pContainer))
  //  //reinterpret_cast<ArithmeticExpression*>()->StepIterator(*this);
  //  ((ArithmeticExpression*)pContainer)->StepIterator(*this);
  //  //*static_cast<CTokens::iterator*>(&tk) = *static_cast<CTokens::iterator*>(this);
  //  return *this;
  //}

  //////////////////////////////////////////////////////////////////////////

  GXBOOL SYNTAXNODE::GLOB::IsToken() const
  {
    return pNode && pNode->magic != FLAG_OPERAND_MAGIC;
  }

  GXBOOL SYNTAXNODE::GLOB::IsNode() const
  {
    return pNode && pNode->magic == FLAG_OPERAND_MAGIC;
  }

  SYNTAXNODE::FLAGS SYNTAXNODE::GLOB::GetType() const
  {
    if(pNode) {
      return (pNode->magic == FLAG_OPERAND_MAGIC)
        ? FLAG_OPERAND_IS_NODE
        : FLAG_OPERAND_IS_TOKEN;
    }
    else {
      return FLAG_OPERAND_UNDEFINED;
    }
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsToken(TOKEN::T_LPCSTR str) const
  {
    return IsToken() ? (*pTokn == str) : NULL;
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsToken(TOKEN::TChar c) const
  {
    return IsToken() ? (*pTokn == c) : NULL;
  }

  const TOKEN* SYNTAXNODE::GLOB::GetFrontToken() const
  {
    if(IsToken()) {
      return pTokn;
    }
    else if(IsNode()) {
      return &pNode->GetAnyTokenAPB();
    }
    return NULL;
  }

  const TOKEN* SYNTAXNODE::GLOB::GetBackToken() const
  {
    if(IsToken()) {
      return pTokn;
    }
    else if(IsNode()) {
      return &pNode->GetAnyTokenBPA();
    }
    return NULL;
  }

  void SYNTAXNODE::Clear()
  {
    mode = MODE_Undefined;
    pOpcode = NULL;
    Operand[0].ptr = NULL;
    Operand[1].ptr = NULL;
  }

  template<class SYNTAXNODE_T>
  void RecursiveNode(ArithmeticExpression* pParser, SYNTAXNODE_T* pNode, std::function<GXBOOL(SYNTAXNODE_T*, int)> func, int depth /*= 0*/) // 广度优先递归
  {
    if(func(pNode, depth)) {
      if(pNode->Operand[0].GetType() == SYNTAXNODE_T::FLAG_OPERAND_IS_NODE) {
        RecursiveNode(pParser, (SYNTAXNODE_T*)pNode->Operand[0].ptr, func, depth + 1);
      }

      if(pNode->Operand[1].GetType() == SYNTAXNODE_T::FLAG_OPERAND_IS_NODE) {
        RecursiveNode(pParser, (SYNTAXNODE_T*)pNode->Operand[1].ptr, func,
          pNode->mode == SYNTAXNODE_T::MODE_Chain ? depth : depth + 1); // next chain 不增加深度
      }
    }
  }

  b32 SYNTAXNODE::CompareOpcode(CTokens::TChar ch) const
  {
    return pOpcode && *pOpcode == ch;
  }

  b32 SYNTAXNODE::CompareOpcode(CTokens::T_LPCSTR str) const
  {
    return pOpcode && *pOpcode == str;
  }

  clStringA& SYNTAXNODE::ToString(clStringA& str) const
  {
    return GetAnyTokenAPB().ToString(str);
  }

  clStringW& SYNTAXNODE::ToString(clStringW& str) const
  {
    return GetAnyTokenAPB().ToString(str);
  }

#if 0
  VALUE::State SYNTAXNODE::Calcuate(const NameSet& sNameSet, VALUE& value_out) const
  {
    VALUE p[2];
    VALUE::State s = VALUE::State_OK;
    if(mode == MODE_FunctionCall)
    {
      value_out.SetOne();
      return VALUE::State_Call;
    }

    for(int i = 0; i < 2; i++)
    {
      if(Operand[i].IsNode()) {
        s = Operand[i].pNode->Calcuate(sNameSet, p[i]);
      }
      else if(Operand[i].IsToken()) {
        if(Operand[i].pTokn->type == TOKEN::TokenType_Numeric) {
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
    if(TEST_FLAG(s, VALUE::State_UnknownOpcode))
    {
      if(*pOpcode == '.')
      {
        ASSERT(Operand[0].IsToken());
        const TYPEDESC* pTypeDesc = sNameSet.GetVariable(Operand[0].pTokn);
      }
      CLNOP;
    }
    return s;
  }
#endif

  const TOKEN& SYNTAXNODE::GetAnyTokenAB() const // 深度优先
  {
    if(Operand[0].IsToken()) {
      return *Operand[0].pTokn;
    }
    else if(Operand[0].IsNode()) {
      return Operand[0].pNode->GetAnyTokenAB();
    }
    else if(Operand[1].IsToken()) {
      return *Operand[1].pTokn;
    }
    else if(Operand[1].IsNode()) {
      return Operand[1].pNode->GetAnyTokenAB();
    }
    CLBREAK;
  }

  const TOKEN& SYNTAXNODE::GetAnyTokenAB2() const // 广度优先
  {
    if(Operand[0].IsToken()) {
      return *Operand[0].pTokn;
    }
    else if(Operand[1].IsToken()) {
      return *Operand[1].pTokn;
    }
    else if(Operand[0].IsNode()) {
      return Operand[0].pNode->GetAnyTokenAB();
    }
    else if(Operand[1].IsNode()) {
      return Operand[1].pNode->GetAnyTokenAB();
    }
    CLBREAK;
  }

  const TOKEN& SYNTAXNODE::GetAnyTokenAPB() const
  {
    if(Operand[0].IsToken()) {
      return *Operand[0].pTokn;
    }
    else if(Operand[0].IsNode()) {
      return Operand[0].pNode->GetAnyTokenAPB();
    }
    else if(pOpcode) {
      return *pOpcode;
    }
    else if(Operand[1].IsToken()) {
      return *Operand[1].pTokn;
    }
    else if(Operand[1].IsNode()) {
      return Operand[1].pNode->GetAnyTokenAPB();
    }
    CLBREAK;
  }

  const TOKEN& SYNTAXNODE::GetAnyTokenBPA() const
  {
    if(Operand[1].IsToken()) {
      return *Operand[1].pTokn;
    }
    else if(Operand[1].IsNode()) {
      return Operand[1].pNode->GetAnyTokenAPB();
    }
    else if(pOpcode) {
      return *pOpcode;
    }
    else if(Operand[0].IsToken()) {
      return *Operand[0].pTokn;
    }
    else if(Operand[0].IsNode()) {
      return Operand[0].pNode->GetAnyTokenAPB();
    }
    CLBREAK;
  }

  const TOKEN& SYNTAXNODE::GetAnyTokenPAB() const
  {
    if(pOpcode) {
      return *pOpcode;
    }
    else if(Operand[0].IsToken()) {
      return *Operand[0].pTokn;
    }
    else if(Operand[1].IsToken()) {
      return *Operand[1].pTokn;
    }
    else if(Operand[0].IsNode()) {
      return Operand[0].pNode->GetAnyTokenPAB();
    }
    else if(Operand[1].IsNode()) {
      return Operand[1].pNode->GetAnyTokenPAB();
    }
    CLBREAK;
  }

  bool TokenPtr::operator<(const TokenPtr& tp2) const
  {    
    if(ptr->length != tp2.ptr->length) {
      return ptr->length < tp2.ptr->length;
    }
    auto s1 = ptr->marker;
    auto s2 = tp2.ptr->marker;
    for(clsize i = 0; i < ptr->length; i++, s1++, s2++)
    {
      if(*s1 != *s2) {
        return *s1 < *s2;
      }
    }
    return false;
  }

  ArithmeticExpression::iterator& ArithmeticExpression::iterator::operator++()
  {
    pContainer->next(*this);

    tk.ClearMarker();
    tk.ClearArithOperatorInfo();

    ((ArithmeticExpression*)pContainer)->StepIterator(*this);

    //tk.Set(*this);
#ifdef ENABLE_STRINGED_SYMBOL
    ToString(tk.symbol);
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    tk.scope = -1;
    tk.semi_scope = -1;
    tk.pContainer = pContainer;

    tk.marker = marker;
    tk.length = length;
    return *this;
  }

 TOKEN* ArithmeticExpression::iterator::operator->()
  {
    return &tk;
  }

  TOKEN& ArithmeticExpression::iterator::operator*()
  {
    return tk;
  }
  //ArithmeticExpression::iterator& ArithmeticExpression::iterator::operator=(const TOKEN& t)
  //{
  //  *static_cast<TOKEN*>(this) = t;
  //  return *this;
  //}

  //  const TOKEN* ArithmeticExpression::iterator::operator->() const
//  {
//    return &tk;
//  }
//
//  const TOKEN& ArithmeticExpression::iterator::operator*() const
//  {
//    return tk;
//  }
//
//#if 0
//  b32 ArithmeticExpression::iterator::operator==(const _TStr& str) const
//  {
//    return (str.GetLength() == tk.length && !clstd::strncmpT(tk.marker, (T_LPCSTR)str, (int)tk.length));
//  }
//  
//  b32 ArithmeticExpression::iterator::operator==(T_LPCSTR pStr) const
//  {
//    const size_t nStrLength = clstd::strlenT(pStr);
//    return (nStrLength == tk.length && !clstd::strncmpT(tk.marker, pStr, (int)tk.length));
//  }
//
//  b32 ArithmeticExpression::iterator::operator==(TChar ch) const
//  {
//    return (tk.length == 1 && tk.marker[0] == ch);
//  }
//
//  b32 ArithmeticExpression::iterator::operator!=(const _TStr& str) const
//  {
//    return (str.GetLength() != tk.length || clstd::strncmpT(tk.marker, (T_LPCSTR)str, (int)tk.length));
//  }
//
//  b32 ArithmeticExpression::iterator::operator!=(T_LPCSTR pStr) const
//  {
//    return (_TStr(pStr).GetLength() != tk.length || clstd::strncmpT(tk.marker, pStr, (int)tk.length));
//  }
//
//  b32 ArithmeticExpression::iterator::operator!=(TChar ch) const
//  {
//    return (tk.length != 1 || tk.marker[0] != ch);
//  }
//
//
//  b32 ArithmeticExpression::iterator::operator==(const iterator& it) const
//  {
//    ASSERT(tk.pContainer == it.tk.pContainer);
//    return (tk.marker == it.tk.marker && tk.length == it.tk.length);
//  }
//  
//  b32 ArithmeticExpression::iterator::operator!=(const iterator& it) const
//  {
//    return (!operator==(it));
//  }
//  
//  b32 ArithmeticExpression::iterator::operator>=(const iterator& it) const
//  {
//    ASSERT(tk.pContainer == it.tk.pContainer);
//    return (tk.marker > it.tk.marker) ||
//      (tk.marker == it.tk.marker && tk.length == it.tk.length);
//  }
//
//  b32 ArithmeticExpression::iterator::operator<=(const iterator& it) const
//  {
//    ASSERT(tk.pContainer == it.tk.pContainer);
//    return (tk.marker < it.tk.marker) ||
//      (tk.marker == it.tk.marker && tk.length == it.tk.length);
//  }
//
//  TOKEN::T_LPCSTR ArithmeticExpression::iterator::begin() const
//  {
//    return tk.marker;
//  }
//
//  TOKEN::T_LPCSTR ArithmeticExpression::iterator::end() const
//  {
//    return tk.marker + tk.length;
//  }
//
//#endif
//
  ArithmeticExpression::iterator& ArithmeticExpression::iterator::operator=(const TOKEN& token)
  {
    //ASSERT(&token != &tk);
    tk = token;
    *static_cast<CTokens::iterator*>(this) = *static_cast<const CTokens::iterator*>(&token);
    return *this;
  }

  ArithmeticExpression::iterator::iterator()
  {
    tk.ClearMarker();
    tk.ClearArithOperatorInfo();
  }

  ArithmeticExpression::iterator::iterator(const TOKEN& token)
  {
    this->operator=(token);
  }
//

//b32 ArithmeticExpression::iterator::operator!=(const ArithmeticExpression::iterator& it) const
//{
//  return CTokens::iterator::operator!=(it);
//}

} // namespace UVShader