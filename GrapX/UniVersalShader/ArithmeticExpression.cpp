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
// HLSL不支持这个，uvs不支持这个。
// 3.GLSL支持“^^”符号，代表布尔异或，uvs不支持，查了一下，HLSL也没有这个符号。
// 4.GLSL支持数组的length()方法，uvs支持这个方法
// 5.GLSL支持数组初始化：
//  （1）vec4 Scene[] = vec4[](vec4(0, 0, 0, 1), vec4(1, 1, 1, 1), vec4(2, 2, 2, 1));
//  （2）const float [] weights = float [] (	0.44198,	0.27901);
//  （3）sph[] lights = sph[] (
//         sph(v30, 0., 0, 0),
//         sph(vec3(-2., 2., 3.), .1, _wht_e, _lit1),
//         sph(vec3(2., 3., -3.), .5, _rnbw_e, _lit2)
//         ); // sph是结构体
// 5.1 基于以上，GLSL支持在声明数组后在其后给数组赋值：weights = {0.5, 0.5};
//     但是HLSL和C++是不能这样给数组赋值的
// HLSL不支持，uvs不打算支持这种形式。
// 6.GLSL支持复杂表达式声明数组长度，如：
//  const vec2 t= vec2(2,3);
//  const float c[int(t.x) * int(t.y)];
// HLSL不支持这种声明，uvs也暂时不打算支持这种声明
// 7.HLSL向量/矩阵比较结果是bool向量类型，GLSL比较结果是bool类型
// 8.GLSL支持表达式直接使用初始化列表，如“{1,2,3,4,5}[1]”结果是“2”，“float pos = {-0.5,-0.25,0.25,0.5}[int(rtime*br2)%4]*2.;”
// 9.GLSL支持“struct Ray{vec3 o,d;},_ray;”这样的语法，没测试HLSL是否支持，C++不支持。
// 10.GLSL有#elseif预处理指令，HLSL似乎只有#elif
// 11.HLSL 不支持switch case语法：
//     int n = 0, m = 0;
//     switch(n)
//     {
//     case 0: n = 2; break;
//     case 1: n = 2; if(m == 1) { case 2: n = 3; } break;
//     }
// 12.GLSL 支持“#ifndef RAY_MODE && DEMO_MODE”这样的预处理，HLSL和C++不支持
// 13.GLSL和HLSL对于预处理检查比较宽松，比如“#define ANTIALIASING;”(后面有个多余分号)或者“#define PIF.14159”(PIF与定义数字之间没有空格)是没问题的，C++会报错
// 14.case标签下可以初始化变量，这在C++语法里是不行的
// 15.GLSL结构体定义可以使用const修饰，HLSL在结构体没有定义变量时是不能使用const修饰的。
// 16.GLSL结构体结尾可以没有分号，HLSL不行

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
    : m_NodePool(128)
    , m_bHigherDefiniton(FALSE)
    , m_pLogger(NULL)
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
        //pThis->GetLogger()->WriteErrorW(TRUE, it.offset(), UVS_EXPORT_TEXT2(1071, "在注释中遇到意外的文件结束", pThis));
        pThis->GetLogger()->OutputErrorW(it.marker, UVS_EXPORT_TEXT(1071, "在注释中遇到意外的文件结束"));
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
    return it;
  }

  ArithmeticExpression::iterator ArithmeticExpression::end()
  {
    CTokens::iterator it_base = CTokens::end();
    ArithmeticExpression::iterator it;
    it.marker = it_base.marker;
    it.length = it_base.length;
    it.pContainer = it_base.pContainer;
    return it;
  }

  const SYNTAXNODE* ArithmeticExpression::TryGetNode(const GLOB* pDesc) const
  {
    if(pDesc->IsNode()) {
      return pDesc->pNode;
    }
    return NULL;
  }

  SYNTAXNODE::MODE ArithmeticExpression::TryGetNodeMode(const GLOB* pDesc) const
  {
    if(pDesc->IsNode()) {
      return pDesc->pNode->mode;
    }
    return SYNTAXNODE::MODE_Undefined;
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(GLOB* pDest, SYNTAXNODE::MODE mode, GLOB* pOperandA, GLOB* pOperandB)
  {
    return MakeSyntaxNode(pDest, mode, NULL, pOperandA, pOperandB);
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(GLOB* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, GLOB* pOperandA, GLOB* pOperandB)
  {
    SYNTAXNODE sNode;
    sNode.magic   = SYNTAXNODE::FLAG_OPERAND_MAGIC;
    sNode.mode    = mode;
    sNode.pOpcode = pOpcode;
    sNode.Operand[0].ptr = (pOperandA == NULL) ? NULL : pOperandA->ptr;
    sNode.Operand[1].ptr = (pOperandB == NULL) ? NULL : pOperandB->ptr;

    // 验证一元操作符不会出现在操作数中
    ASSERT(pOperandA == NULL || pOperandA->ptr == NULL || pOperandA->IsNode() || (
      (*pOperandA->pTokn) != '~' && (*pOperandA->pTokn) != '!' &&
      (*pOperandA->pTokn) != '-' && (*pOperandA->pTokn) != '+' &&
      (*pOperandA->pTokn) != "--" && (*pOperandA->pTokn) != "++"));

    pDest->pNode = m_NodePool.PushBack(sNode);

#ifdef ENABLE_SYNTAX_NODE_ID
    pDest->pNode->id = m_nNodeId++;
#endif

    return TRUE;
  }

  GXBOOL ArithmeticExpression::MakeInstruction(int depth, const TOKEN* pOpcode, int nMinPrecedence, const TKSCOPE* pScope, GLOB* pParent, int nMiddle)
  {
    ASSERT((int)pScope->begin <= nMiddle);
    ASSERT(nMiddle <= (int)pScope->end);

    TKSCOPE scopeA(pScope->begin, nMiddle);
    TKSCOPE scopeB(nMiddle + 1, pScope->end);
    GLOB A = {0}, B = {0};
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
        bresult = bresult && MakeInstruction(depth + 1, &m_aTokens[s.scope], TOKEN::FIRST_OPCODE_PRECEDENCE, &scopeB, &B, s.scope);
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
      MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Assignment, NULL, &A, &B);
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
      clStringW strW;
      /*if(A.pNode != NULL && B.pNode != NULL)
      {
        // ERROR: 一元操作符不能同时带有左右操作数
        return FALSE;
      }
      else */if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_LEFT_OPERAND) && A.pNode != NULL)
      {
        // ERROR: 一元操作符不接受左值
        GetLogger()->OutputErrorW(pOpcode, UVS_EXPORT_TEXT(5075, "“%s”：一元操作符不接受左值"), pOpcode->ToString(strW).CStr());
        return FALSE;
      }
      else if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_RIGHT_OPERAND) && B.pNode != NULL)
      {
        // ERROR: 一元操作符不接受右值
        GetLogger()->OutputErrorW(pOpcode, UVS_EXPORT_TEXT(5076, "“%s”：一元操作符不接受右值"), pOpcode->ToString(strW).CStr());
        return FALSE;
      }
    }

    return bresult;
  }

  GXBOOL ArithmeticExpression::IsLikeTypeCast(const TKSCOPE& scope, TKSCOPE::TYPE i)
  {
    const TOKEN& front = m_aTokens[scope.begin];
    if(front == '(' && (scope.begin == i || front.scope == i) && (TKSCOPE::TYPE)front.scope + 1 < scope.end) // (...)... 形式
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

  ArithmeticExpression::TKSCOPE::TYPE ArithmeticExpression::FindComma(const TKSCOPE& scope)
  {
    for(TKSCOPE::TYPE i = scope.begin; i < scope.end; ++i)
    {
      if(m_aTokens[i].scope != -1) {
        i = m_aTokens[i].scope;
      }
      else if(m_aTokens[i] == ',') {
        return i;
      }
    }
    return TKSCOPE::npos;
  }

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
          const TOKEN& s = m_aTokens[i];

          if(s.precedence == TOKEN::ID_BRACE) // 跳过非运算符, 也包括括号
          {
            ASSERT(s.scope < (int)scope.end); // 闭括号肯定在表达式区间内
            if(nCandidate > OPP(12) && IsLikeTypeCast(scope, i)) {
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
            if(nCandidate > OPP(12) && IsLikeTypeCast(scope, i)) {
              return i;
            }
            i = s.scope;
            continue;
          }
          else if(s.precedence == OPP(1) && s.scope != -1)
          {
            ASSERT(s == ':');
            i = s.scope;
            nCandidate = s.precedence;
            nCandidatePos = i;
            continue;
          }
          else if(s.precedence == 0) { // 跳过非运算符
            continue;
          }

          if(s.precedence == nMinPrecedence) {
            return i;
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

  SYNTAXNODE* ArithmeticExpression::AllocNode(SYNTAXNODE::MODE m, void* pOperand0, void* pOperand1)
  {
    SYNTAXNODE* pNewNode = m_NodePool.Alloc();
#ifdef ENABLE_SYNTAX_NODE_ID
    pNewNode->id = m_nNodeId++;
#endif
    pNewNode->magic   = SYNTAXNODE::FLAG_OPERAND_MAGIC;
    pNewNode->mode    = m;
    pNewNode->pOpcode = NULL;
    pNewNode->Operand[0].ptr = pOperand0;
    pNewNode->Operand[1].ptr = pOperand1;
    return pNewNode;
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, const TKSCOPE& scope, GLOB* pDesc)
  {
    return ParseArithmeticExpression(depth + 1, scope, pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, const TKSCOPE& scope, GLOB* pDesc, int nMinPrecedence)
  {
    GLOB A, B;

    if(depth > 1000)
    {
      // ERROR: 表达式解析堆栈不足
      //m_pMsg->WriteErrorW(TRUE, m_aTokens[scope.begin].offset(), UVS_EXPORT_TEXT(5001, "表达式解析堆栈不足"));
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5001, "表达式解析堆栈不足"));
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

      if(A.pTokn->precedence == TOKEN::ID_BRACE && A.pTokn->scope == scope.begin + 1) {
        if(*A.pTokn == '{') {
          bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_InitList, A.pTokn, NULL, NULL);
        }
        else {
          bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Bracket, A.pTokn, &A, &B);
        }
      }
      else if(A.pTokn->precedence > OPP(0)) // 不包括“,”
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, A.pTokn, NULL, &B);
        DbgDumpScope(A.pTokn->ToString(), TKSCOPE(0,0), TKSCOPE(scope.begin + 1, scope.end));
      }
      else if(B.pTokn->precedence > OPP(0))
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, B.pTokn, &A, NULL);
        DbgDumpScope(B.pTokn->ToString(), TKSCOPE(scope.begin, scope.begin + 1), TKSCOPE(0,0));
      }
      else if(front.IsIdentifier()) {
        // 变量声明
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Definition, &A, &B);
      }
      else {
        goto GO_NEXT;
      }
      return bret;
GO_NEXT:;
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
      if(front == '{') {
        GXBOOL bRet = ParseArithmeticExpression(depth + 1, TKSCOPE(scope.begin + 1, scope.end - 1), pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
        if(bRet) {
          bRet = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_InitList, pDesc, NULL);
        }
        return bRet;
      }
      else {
        return ParseArithmeticExpression(depth + 1, TKSCOPE(scope.begin + 1, scope.end - 1), pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
      }
    }
    else if(front.IsIdentifier() && m_aTokens[scope.begin + 1].scope == scope.end - 1)  // 整个表达式是函数调用
    {
      // X(...) 形式
      return ParseFunctionCall(scope, pDesc);
    }

#ifdef REFACTOR_COMMA
    GXBOOL bret = BreakComma(depth, scope, pDesc, nMinPrecedence);
    if(bret >= 0) {
      return bret;
    }
#endif

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
    return ParseFunctionSubscriptCall(scope, pDesc);
  }

  GXBOOL ArithmeticExpression::BreakComma(int depth, const TKSCOPE& scope, GLOB* pDesc, int nMinPrecedence)
  {
    //GLOB A;
    GLOB B;
    // 对“,”操作符进行特殊处理，以便支持大超大数组
    if(nMinPrecedence == OPP(0))
    {
      TKSCOPE scopeB = scope;
      B.ptr = NULL;
      struct GLOB_COMMA
      {
        GLOB A;
        TKSCOPE scopeA;
        TOKEN* pComma;
      };
      GLOB_COMMA gc;

      clstack<GLOB_COMMA> sGlobStack;
      while(true)
      {
        TKSCOPE::TYPE nComma = FindComma(scopeB);
        if(nComma == TKSCOPE::npos)
        {
          if(sGlobStack.empty()) {
            return -1;
          }

          if(ParseArithmeticExpression(depth + 1, scopeB, pDesc, nMinPrecedence + 1) == FALSE) {
            return FALSE;
          }
          else if(pDesc->ptr == NULL)
          {
            pDesc->ptr = sGlobStack.top().A.ptr;
            sGlobStack.pop();
            if(sGlobStack.empty()) {
              return TRUE;
            }
          }
          else {
            ASSERT(gc.A.ptr == sGlobStack.top().A.ptr);
          }

          do {
            //A = sGlobStack.top();
            B = *pDesc;
            GLOB_COMMA& top_gc = sGlobStack.top();
            MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, top_gc.pComma, &top_gc.A, &B);
            if(sGlobStack.size() < 128)
            {
              DbgDumpScope(top_gc.pComma->ToString(), top_gc.scopeA, scopeB);
            }
            scopeB.begin = top_gc.scopeA.begin;

            sGlobStack.pop();
          } while(_CL_NOT_(sGlobStack.empty()));
          return TRUE;
        }

        TKSCOPE scopeA(scopeB.begin, nComma);
        scopeB.begin = nComma + 1;
        gc.A.ptr = NULL;

        if(ParseArithmeticExpression(depth + 1, scopeA, &gc.A, nMinPrecedence + 1) == FALSE) {
          return FALSE;
        }
        else if(gc.A.ptr == NULL) {
          ERROR_MSG__MISSING_SEMICOLON(m_aTokens[scopeA.begin]);
          return FALSE;
        }
        gc.pComma = &m_aTokens[nComma];
        gc.scopeA = scopeA;
        sGlobStack.push(gc);
      }
    }
    return -1;
  }

  CLogger* ArithmeticExpression::GetLogger()
  {
    return m_pLogger;
  }

  const CLogger* ArithmeticExpression::GetLogger() const
  {
    return m_pLogger;
  }

  GXBOOL ArithmeticExpression::DbgHasError(int errcode) const
  {
    return GetLogger()->HasError(errcode);
  }

  size_t ArithmeticExpression::DbgErrorCount() const
  {
    return GetLogger()->ErrorCount(TRUE);
  }

  GXBOOL ArithmeticExpression::ParseFunctionSubscriptCall(const TKSCOPE& scope, GLOB* pDesc)
  {
    // 从右到左解析这两种形式:
    // name(...)(...)(...)
    // name[...][...][...]
    // 括号域之间不能有其他符号, 括号域之内是数学表达式

    struct CONTEXT
    {
      SYNTAXNODE::MODE mode;
      GLOB B;
    };

    typedef clstack<CONTEXT> SyntaxStack;
    SyntaxStack node_stack;
    TKSCOPE::TYPE nBracketLimit = scope.begin + 1;
    GLOB A;
    CONTEXT c;
    TOKEN* pBack = &m_aTokens[scope.end - 1];
    A = m_aTokens[scope.begin];

    if(*A.pTokn == '(' && (TKSCOPE::TYPE)(A.pTokn->scope + 1) < scope.end && // 类似"()[]"或者"()[][]"形式
      m_aTokens[A.pTokn->scope + 1].scope != TKSCOPE::npos)
    {
      nBracketLimit = A.pTokn->scope + 1;
      if(_CL_NOT_(ParseArithmeticExpression(0, TKSCOPE(scope.begin, nBracketLimit), &A))) {
        return FALSE;
      }
    }
    else if(A.pTokn->precedence != 0)
    {
      clStringW strW;
      GetLogger()->OutputErrorW(A.pTokn, UVS_EXPORT_TEXT(5040, "语法错误: “%s”"), A.pTokn->ToString(strW).CStr());
      return FALSE;
    }

    // 从后往前拆解括号
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
        GetLogger()->OutputErrorW(A.pTokn, UVS_EXPORT_TEXT(2054, "在“%s”之后应输入“%s”"), A.pTokn->ToString(str).CStr(), _CLTEXT("="));
        c.mode = SYNTAXNODE::MODE_Undefined;
      }

      c.B.ptr = NULL;

      if(_CL_NOT_(ParseArithmeticExpression(0, TKSCOPE(pBack->scope + 1, pBack - &m_aTokens.front()), &c.B))) {
        return FALSE;
      }

      if(nBracketLimit == pBack->scope) {
        break;
      }
      else {
        node_stack.push(c);
        pBack = &m_aTokens[pBack->scope - 1];
      }
    }

    while(1) {
      if(_CL_NOT_(MakeSyntaxNode(pDesc, c.mode, &A, &c.B))) {
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

  GXBOOL ArithmeticExpression::ParseFunctionCall(const TKSCOPE& scope, GLOB* pDesc)
  {
    // 括号肯定是匹配的
    ASSERT(m_aTokens[scope.end - 1].scope == scope.begin + 1);

    GLOB A, B = {0};
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

  GXBOOL ArithmeticExpression::ParseTypeCast(const TKSCOPE& scope, GLOB* pDesc)
  {
    ASSERT(scope.begin < scope.end - 2); // 这个由之前的判断保证, 目前括号里只可能有一个类型标识符
    //ASSERT(m_aTokens[scope.begin].scope == scope.end - 2); // 外部保证是(A)B 形式

    GLOB A = {0}, B = {0};
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
      //m_pMsg->WriteErrorW(TRUE, m_aTokens[type_scope.begin].offset(), UVS_EXPORT_TEXT(5007, "类型转换:类型无法解析."));
      GetLogger()->OutputErrorW(m_aTokens[type_scope.begin], UVS_EXPORT_TEXT(5007, "类型转换:类型无法解析."));
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
          else {
            if(_CL_NOT_(bSilent)) {
              // ERROR: 括号不匹配
              clStringW str((clStringW::TChar)c.chOpen, 1);
              //m_pMsg->WriteErrorW(TRUE, token.offset(), UVS_EXPORT_TEXT(2059, "括号不匹配, 缺少\"%s\"."), str.CStr());
              GetLogger()->OutputErrorW(token, UVS_EXPORT_TEXT(2059, "括号不匹配, 缺少\"%s\"."), str.CStr());
            }
            token.type = TOKEN::TokenType_Bracket;
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
        token.type = TOKEN::TokenType_Bracket;
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

  clStringArrayA& ArithmeticExpression::DbgGetExpressionStack()
  {
    return m_aDbgExpressionOperStack;
  }

  ArithmeticExpression::T_LPCSTR ArithmeticExpression::GetOriginPtr(const TOKEN* pToken) const
  {
    return NULL;
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

  VALUE& VALUE::SetZero()
  {
    uValue64 = 0;
    rank = Rank_Signed;
    return *this;
  }

  VALUE& VALUE::SetOne()
  {
    uValue64 = 1;
    rank = Rank_Signed;
    return *this;
  }

  GXBOOL VALUE::IsZero() const
  {
    switch(rank)
    {
    case UVShader::VALUE::Rank_Unsigned:
      return (uValue == 0);
    case UVShader::VALUE::Rank_Signed:
      return (nValue == 0);
    case UVShader::VALUE::Rank_Float:
      return (fValue == 0);
    case UVShader::VALUE::Rank_Unsigned64:
      return (uValue64 == 0);
    case UVShader::VALUE::Rank_Signed64:
      return (nValue64 == 0);
    case UVShader::VALUE::Rank_Double:
      return (fValue64 == 0);
    case UVShader::VALUE::Rank_String:
      CLBREAK;
      break;
    default:
      CLBREAK;
      break;
    }
    return FALSE;
  }

  GXBOOL VALUE::IsNegative() const
  {
    switch(rank)
    {
    case UVShader::VALUE::Rank_Unsigned:
    case UVShader::VALUE::Rank_Unsigned64:
      return FALSE;
    case UVShader::VALUE::Rank_Signed:
      return (nValue < 0);
    case UVShader::VALUE::Rank_Float:
      return (fValue < 0);
    case UVShader::VALUE::Rank_Signed64:
      return (nValue64 < 0);
    case UVShader::VALUE::Rank_Double:
      return (fValue64 < 0);
    case UVShader::VALUE::Rank_String:
      CLBREAK;
      break;
    default:
      CLBREAK;
      break;
    }
    return FALSE;
  }

  template<typename _Ty, typename _PartT>
  _Ty MakeFloatValueWithExp(const _PartT* digi, GXBOOL bNegative, GXBOOL bNegExp)
  {
    _Ty fValue = (_Ty)digi[1];
    while(fValue > _Ty(1.0)) {
      fValue *= _Ty(0.1);
    }
    fValue += digi[0];

    if(digi[2] != 0) {
      if(bNegExp) {
        for(size_t i = 0; i < digi[2]; i++) {
          fValue *= _Ty(0.1);
        }
      }
      else {
        for(size_t i = 0; i < digi[2]; i++) {
          fValue *= _Ty(10.0);
        }
      }
    }

    if(bNegative) {
      fValue = -fValue;
    }
    return fValue;
  }

  VALUE& VALUE::set(const VALUE& v)
  {
    *this = v;
    return *this;
  }

  VALUE::State VALUE::set(const TOKEN& token)
  {
    if(token.type == TOKEN::TokenType_Integer) {
      return set(token.marker, token.length, TRUE);
    }
    State s = set(token.marker, token.length, FALSE);
    if(s == State_Overflow) {
      return SetAsFloat(token);
    }
    return s;
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
    //if(ptr[count - 1] == 'f' || ptr[count - 1] == 'F') {
    //  SET_FLAG(dwFlags, Rank_Float);
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

    if(ptr[i] == '0' && i + 1 < count && (ptr[i + 1] == 'X' || ptr[i + 1] == 'x'))
    {
      i += 2;
      if(i >= count) {
        return State_IllegalChar;
      }

      int n;
      for(; i < count; i++)
      {
        if(ptr[i] >= '0' && ptr[i] <= '9') {
          n = ptr[i] - '0';
        }
        else if(ptr[i] >= 'A' && ptr[i] <= 'F') {
          n = ptr[i] - ('A' - 10);
        }
        else if(ptr[i] >= 'a' && ptr[i] <= 'f') {
          n = ptr[i] - ('a' - 10);
        }
        else if((ptr[i] == 'U' || ptr[i] == 'u') && i == count - 1) {
          ASSERT(dwFlags == Rank_Unsigned || dwFlags == Rank_Unsigned64);
          break;
        }
        else {
          return State_IllegalChar;
        }
        // FIXME: 没有判断溢出情况
        digi[p] = digi[p] * 16 + n;
      }
    }
    else if(ptr[i] == '0' && bInteger) // 8进制
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
        else if((ptr[i] == 'U' || ptr[i] == 'u') && i == count - 1) {
          ASSERT(dwFlags == Rank_Unsigned || dwFlags == Rank_Unsigned64);
          break;
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
        else if(i == count - 1 && p == 0 && (ptr[i] == 'u' || ptr[i] == 'U'))
        {
          dwFlags = Rank_Unsigned;
          break;
        }
        else if(i == count - 1 && p > 0 && (ptr[i] == 'f' || ptr[i] == 'F'))
        {
          dwFlags = Rank_Float;
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
#ifdef ENABLE_HIGH_PRECISION_FLOAT
      fValue64 = MakeFloatValueWithExp<double>(digi, ptr[0] == '-', bNegExp);
      SET_FLAG(dwFlags, Rank_F_LongLong);
#else
      uValue64 = 0; // 清除高32位
      fValue = MakeFloatValueWithExp<float>(digi, ptr[0] == '-', bNegExp);
#endif
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

  VALUE::State VALUE::SetAsFloat(const TOKEN& token)
  {
#ifdef ENABLE_HIGH_PRECISION_FLOAT
    return SetAsFloat<double>(token.marker, token.length);
#else
    return SetAsFloat<float>(token.marker, token.length);
#endif
  }

  template<typename _RealT>
  VALUE::State VALUE::SetAsFloat(TOKEN::T_LPCSTR ptr, size_t count)
  {
    // X是任意内容，D是指数字
    // "-X" "+X" ".X" "Xf"
    // "De-D" "DeD"

    GXDWORD dwFlags = 0;
    _RealT digi[3] = { 0 }; // [0]是整数部分，[1]是小数部分, [2]是指数
    size_t p = 0; // part
    size_t i = 0;
    GXBOOL bNegExp = FALSE;
    ASSERT(count != 0); // 不可能为空

    rank = Rank_BadValue;
    SET_FLAG(dwFlags, sizeof(_RealT) == sizeof(float) ? Rank_Float : Rank_Double);

    // FIXME: 这种写法会认为"10f"为合法的,实际C/C++中不承认这种写法
    //if(ptr[count - 1] == 'f' || ptr[count - 1] == 'F') {
    //  SET_FLAG(dwFlags, Rank_Float);
    //  count--;
    //}

    // 符号解析
    if(ptr[i] == '-' || ptr[i] == '+') {
      i++;
    }

    while(ptr[i] == 0x20 || ptr[i] == '\t' || ptr[i] == '\r' || ptr[i] == '\n') {
      if(++i >= count) { // 只有一个+/-符号
        return State_SyntaxError;
      }
    }

    for(; i < count; i++)
    {
      if(ptr[i] == '.')
      {
        //SET_FLAG(dwFlags, Rank_Float);
        p++;
        if(p >= 2) { // 不会出现两个‘.’, 这个由TOKEN中的浮点数分析保证
          return State_SyntaxError;
        }
      }
      else if(ptr[i] == 'e' || ptr[i] == 'E')
      {
        p = 2;
        i++;
        //SET_FLAG(dwFlags, Rank_Float);
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
        digi[p] = digi[p] * _RealT(10) + (_RealT)n;
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

    if(dwFlags == Rank_Float)
    {
      uValue64 = 0; // 清除高32位
      fValue = MakeFloatValueWithExp<_RealT>(digi, ptr[0] == '-', bNegExp);
    }

    rank = (Rank)dwFlags;
    return State_OK;
  }

  void VALUE::set(Rank r, const void* pValue)
  {
    rank = r;
    uValue64 = 0; // 清除高32位
    switch(r)
    {
    case VALUE::Rank_Unsigned:
      uValue = *static_cast<const GXUINT*>(pValue);
      break;
    case VALUE::Rank_Signed:
      nValue = *static_cast<const GXINT*>(pValue);
      break;
    case VALUE::Rank_Float:
      fValue = *static_cast<const float*>(pValue);
      break;
    case VALUE::Rank_Unsigned64:
      uValue64 = *static_cast<const GXUINT64*>(pValue);
      break;
    case VALUE::Rank_Signed64:
      nValue64 = *static_cast<const GXINT64*>(pValue);
      break;
    case VALUE::Rank_Double:
      fValue64 = *static_cast<const double*>(pValue);
      break;
    case VALUE::Rank_String:
      uValue64 = reinterpret_cast<const GXUINT64>(pValue);
      break;
    default:
      rank = Rank_Unsigned64;
      CLBREAK;
      break;
    }
  }

  template<typename _Ty>
  VALUE::State VALUE::CalculateT(_Ty& output, const TOKEN::T_LPCSTR szOpcode, size_t nOpcodeLen, const _Ty& t1, const _Ty& t2)
  {
    if(nOpcodeLen == 1)
    {
      switch(szOpcode[0])
      {
      case '+': output = t1 + t2; break;
      case '-': output = t1 - t2; break;
      case '*': output = t1 * t2; break;
      case '<': output = _Ty(t1 < t2); break;
      case '>': output = _Ty(t1 > t2); break;
      case '!': output = _Ty( ! t2); break;
      case '/':
        output = t1 / t2;
        if(t2 == 0) {
          return State_DivideByZeroF;
        }
        break;
      default:
        return State_UnknownOpcode;
        //TRACE("Unsupport opcode(%c).\n", opcode);
        //CLBREAK;
      }
    }
    else if(nOpcodeLen == 2)
    {
      if(szOpcode[0] == '&' && szOpcode[1] == '&') {
        output = (t1 && t2);
      }
      else if(szOpcode[0] == '|' && szOpcode[1] == '|') {
        output = (t1 || t2);
      }
      else if(szOpcode[0] == '=' && szOpcode[1] == '=') {
        output = (t1 == t2);
      }
      else if(szOpcode[0] == '!' && szOpcode[1] == '=') {
        output = (t1 != t2);
      }
      else if(szOpcode[0] == '>' && szOpcode[1] == '=') {
        output = _Ty(t1 >= t2);
      }
      else if(szOpcode[0] == '<' && szOpcode[1] == '=') {
        output = _Ty(t1 <= t2);
      }
      else {
        return State_UnknownOpcode;
        //TRACE("Unsupport opcode(%c).\n", opcode);
        //CLBREAK;
      }
    }

    return State_OK;
  }

  template<typename _Ty>
  VALUE::State VALUE::CalculateIT(_Ty& output, const TOKEN::T_LPCSTR szOpcode, size_t nOpcodeLen, const _Ty& t1, const _Ty& t2)
  {
    // [整数限定操作符]
    if(nOpcodeLen == 1)
    {
      switch(szOpcode[0])
      {
      case '%': output = t1 % t2; break;
      case '^': output = t1 ^ t2; break;
      case '&': output = t1 & t2; break;
      case '|': output = t1 | t2; break;
      case '~': output = (~t2); break;
      case '/':
        if(t2 == 0) {
          output = ~(_Ty)0;
          return State_DivideByZeroI;
        }
        output = t1 / t2;
        break;
      default:
        return State_UnknownOpcode;
      }
    }
    else if(nOpcodeLen == 2)
    {
      if(szOpcode[0] == '<' && szOpcode[1] == '<') {
        output = (t1 << t2);
      }
      else if(szOpcode[0] == '>' && szOpcode[1] == '>') {
        output = (t1 >> t2);
      }
      else {
        return State_UnknownOpcode;
      }
    }

    return State_OK;
  }

  VALUE::State VALUE::Calculate(const TOKEN& token, const VALUE& param0, const VALUE& param1)
  {
    return Calculate(token.marker, token.length, param0, param1);
  }

  VALUE::State VALUE::Calculate(const TOKEN::T_LPCSTR szOpcode, size_t nOpcodeLen, const VALUE& param0, const VALUE& param1)
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
      state = CalculateT(nValue64, szOpcode, nOpcodeLen, nValue64, second.nValue64);      
      if(state == State_UnknownOpcode) {
        state = CalculateIT(nValue64, szOpcode, nOpcodeLen, nValue64, second.nValue64);      
      }
    }
    else if(type == Rank_Unsigned64) {
      state = CalculateT(uValue64, szOpcode, nOpcodeLen, uValue64, second.uValue64);
      if(state == State_UnknownOpcode) {
        state = CalculateIT(uValue64, szOpcode, nOpcodeLen, uValue64, second.uValue64);
      }
    }
    else if(type == Rank_Double) {
      state = CalculateT(fValue64, szOpcode, nOpcodeLen, fValue64, second.fValue64);
    }
    else if(type == Rank_Float) {
      state = CalculateT(fValue, szOpcode, nOpcodeLen, fValue, second.fValue);
    }
    else if(type == Rank_Unsigned) {
      state = CalculateIT(uValue, szOpcode, nOpcodeLen, uValue, second.uValue);
      if(state == State_UnknownOpcode) {
        state = CalculateT(uValue, szOpcode, nOpcodeLen, uValue, second.uValue);
      }
    }
    else if(type == Rank_Signed) {
      state = CalculateIT(nValue, szOpcode, nOpcodeLen, nValue, second.nValue);
      if(state == State_UnknownOpcode) {
        state = CalculateT(nValue, szOpcode, nOpcodeLen, nValue, second.nValue);
      }
    }
    else {
      CLBREAK;
      return State_SyntaxError;
    }
    rank = type;
    return state;
  }

  clStringA& VALUE::ToString(clStringA& str) const
  {
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
    case Rank_Float:
      str.AppendFormat("%.3f", fValue);
      break;
    case Rank_Double:
      str.AppendFloat((float)fValue64);
      break;
    default:
      CLBREAK;
    }
    return str;
  }

  clStringA VALUE::ToString() const
  {
    clStringA str;    
    return ToString(str);
  }

  GXBOOL VALUE::IsNumericRank() const
  {
    return IsNumericRank(rank);
  }

  GXBOOL VALUE::IsNumericRank(Rank _rank)
  {
    return (_rank >= 0 && _rank <= 7);
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
      CLBREAK; // bad rank
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
    type   = TokenType_Identifier;
    bPhony = 0;
  }

  void TOKEN::Set(clstd::StringSetA& sStrSet, const clStringA& str)
  {
    ASSERT(str.IsNotEmpty());
    ASSERT(type == TokenType_String || operator==("__LINE__"));

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
    ASSERT(type == TokenType_String || operator==("true") || operator==("false"));

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
    // TODO: 断言验证type与marker内容的一致性
    // TODO: 以后可能会简化这个判断改为直接返回type
    if(length < 1) {
      ASSERT(type != TokenType_Identifier);
      return FALSE;
    }

    if(_CL_NOT_(marker[0] == '_' ||
      (marker[0] >= 'A' && marker[0] <= 'Z') ||
      (marker[0] >= 'a' && marker[0] <= 'z') ))
    {
      ASSERT(type != TokenType_Identifier);
      return FALSE;
    }
    
    for(clsize i = 1; i < length; i++)
    {
      if(_CL_NOT_(marker[0] == '_' ||
        (marker[0] >= 'A' && marker[0] <= 'Z') ||
        (marker[0] >= 'a' && marker[0] <= 'z') ||
        (marker[0] >= '0' && marker[0] <= '9') ))
      {
        ASSERT(type != TokenType_Identifier);
        return FALSE;
      }
    }
    ASSERT(type == TokenType_Identifier || type == TokenType_FormalParams);
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

  b32 TOKEN::HasReplacedValue() const
  {
    return (bPhony && type == TokenType_Identifier);
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL SYNTAXNODE::GLOB::IsToken() const
  {
    return (pNode && (pNode->magic & FLAG_OPERAND_MAGIC_REPLACED) != FLAG_OPERAND_MAGIC_REPLACED);
  }

  GXBOOL SYNTAXNODE::GLOB::IsNode() const
  {
    return (pNode && (pNode->magic & FLAG_OPERAND_MAGIC_REPLACED) == FLAG_OPERAND_MAGIC_REPLACED);
  }

  GXBOOL SYNTAXNODE::GLOB::IsReplaced() const
  {
    if(IsToken()) {
      return pTokn->HasReplacedValue();
    }
    else if(IsNode()) {
      return (pNode && (pNode->magic == FLAG_OPERAND_MAGIC_REPLACED));
    }
    return FALSE;
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsToken(TOKEN::T_LPCSTR str) const
  {
    return IsToken() ? (*pTokn == str) : FALSE;
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsToken(TOKEN::TChar c) const
  {
    return IsToken() ? (*pTokn == c) : FALSE;
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsNode(MODE _mode) const
  {
    return (IsNode() && pNode->mode == _mode);
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsNode(CTokens::TChar ch) const
  {
    return (IsNode() && pNode->CompareOpcode(ch));
  }

  GXBOOL SYNTAXNODE::GLOB::CompareAsNode(CTokens::T_LPCSTR str) const
  {
    return (IsNode() && pNode->CompareOpcode(str));
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

  clStringA& SYNTAXNODE::GLOB::ToString(clStringA& str) const
  {
    if(IsToken()) {
      pTokn->ToString(str);
    }
    else if(IsNode()) {
      pNode->ToString(str);
    }
    else {
      str = "(null ptr)";
    }
    return str;
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
      if(pNode->Operand[0].IsNode()) {
        RecursiveNode(pParser, (SYNTAXNODE_T*)pNode->Operand[0].ptr, func, depth + 1);
      }

      if(pNode->Operand[1].IsNode()) {
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
    clStringA strOp;
    clStringA strOper[2];
    switch(mode)
    {
    case UVShader::SYNTAXNODE::MODE_Opcode:
      if(Operand[0].ptr) {
        Operand[0].ToString(str).Append(pOpcode->ToString(strOp));
        if(Operand[1].ptr) {
          Operand[1].ToString(strOper[1]);
          str.Append(strOper[1]);
        }
      }
      else if(Operand[1].ptr) {
        pOpcode->ToString(str).Append(Operand[1].ToString(strOper[1]));
      }
      break;
    case UVShader::SYNTAXNODE::MODE_InitList:
      ASSERT(Operand[1].ptr == NULL);
      Operand[0].ToString(strOper[0]);
      str.Reserve(strOper[0].GetLength() + 2);
      str = "{";
      str.Append(strOper[0]).Append("}");
      break;
    case UVShader::SYNTAXNODE::MODE_FunctionCall:
      Operand[0].ToString(str);
      if(Operand[1].ptr) {
        str.Append("(").Append(Operand[1].ToString(strOper[1])).Append(")");
      }
      else {
        str.Append("()");
      }
      break;
    case UVShader::SYNTAXNODE::MODE_Subscript:
      Operand[0].ToString(str).Append('[').Append(Operand[1].ToString(strOper[1])).Append(']');
      break;
    case UVShader::SYNTAXNODE::MODE_Undefined:
    case UVShader::SYNTAXNODE::MODE_Assignment:
    case UVShader::SYNTAXNODE::MODE_TypeCast:
    case UVShader::SYNTAXNODE::MODE_Typedef:
    case UVShader::SYNTAXNODE::MODE_Subscript0:
    case UVShader::SYNTAXNODE::MODE_Definition:
    case UVShader::SYNTAXNODE::MODE_Bracket:
    case UVShader::SYNTAXNODE::MODE_StructDef:
    case UVShader::SYNTAXNODE::MODE_Flow_While:
    case UVShader::SYNTAXNODE::MODE_Flow_If:
    case UVShader::SYNTAXNODE::MODE_Flow_ElseIf:
    case UVShader::SYNTAXNODE::MODE_Flow_Else:
    case UVShader::SYNTAXNODE::MODE_Flow_For:
    case UVShader::SYNTAXNODE::MODE_Flow_ForInit:
    case UVShader::SYNTAXNODE::MODE_Flow_ForRunning:
    case UVShader::SYNTAXNODE::MODE_Flow_DoWhile:
    case UVShader::SYNTAXNODE::MODE_Flow_Break:
    case UVShader::SYNTAXNODE::MODE_Flow_Continue:
    case UVShader::SYNTAXNODE::MODE_Flow_Case:
    case UVShader::SYNTAXNODE::MODE_Flow_Discard:
    case UVShader::SYNTAXNODE::MODE_Return:
    case UVShader::SYNTAXNODE::MODE_Block:
    case UVShader::SYNTAXNODE::MODE_Chain:
    default:
      CLBREAK;
      break;
    }
    return str;
    //return GetAnyTokenAPB().ToString(str);
  }

  //clStringW& SYNTAXNODE::ToString(clStringW& str) const
  //{
  //  return GetAnyTokenAPB().ToString(str);
  //}

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

  //////////////////////////////////////////////////////////////////////////

  CLogger::CLogger()
    : m_uRefCount(1)
    , m_pMsg(NULL)
    , m_nErrorCount(0)
    , m_nWarningCount(0)
    , m_nSessionError(0)
    , m_nDisplayedError(0)
    , m_bWarning(FALSE)
  {
  }

  CLogger::~CLogger()
  {
    ErrorMessage::Destroy(m_pMsg);
  }

  GXUINT CLogger::AddRef()
  {
    return ++m_uRefCount;
  }

  GXUINT CLogger::Release()
  {
    if(--m_uRefCount == 0) {
      delete this;
      return 0;
    }
    return m_uRefCount;
  }

  void CLogger::Initialize(const char* szExpression, clsize nSize, GXLPCWSTR szFilename)
  {
    if(m_pMsg == NULL) {
      m_pMsg = ErrorMessage::Create();
      m_pMsg->LoadErrorMessage(_CLTEXT("uvsmsg.txt"));
      m_pMsg->SetMessageSign('C');
      m_pMsg->PushFile(szFilename);
    }
    m_pMsg->GenerateCurLines(szExpression, nSize);
  }

  void CLogger::Reset()
  {
    m_errorlist.clear();
    m_nErrorCount = 0;
    m_nWarningCount = 0;
    m_nSessionError = 0;
    m_nDisplayedError = 0;
  }

  GXUINT CLogger::SetError(GXUINT err)
  {
    m_nErrorCount++;
    if(TEST_FLAG(err, UVS_WARNING_MASK)) {
      m_nWarningCount++;
    }
    m_nSessionError++;
    if(m_nSessionError <= c_nMaxSessionError) {
      m_nDisplayedError++;
      if(m_nDisplayedError == c_nMaxErrorCount) {
        m_pMsg->WriteErrorW(FALSE, 0, UVS_EXPORT_TEXT2(9997, "错误数量超过%d条，将停止输出错误消息", this), c_nMaxErrorCount);
      }
    }
    m_errorlist.insert(err);

    return err;
  }

  void CLogger::ResetSessionError()
  {
    m_nSessionError = 0;
  }

  void CLogger::SetCurrentFilenameW(GXLPCWSTR szFilename)
  {
    m_pMsg->SetCurrentFilenameW(szFilename);
  }

  GXLPCWSTR CLogger::GetFilenameW(GXUINT idFile) const
  {
    return m_pMsg->GetFilenameW(idFile);
  }

  GXLPCWSTR CLogger::GetFilePathW(GXUINT idFile /*= 0*/) const
  {
    return m_pMsg->GetFilePathW(idFile);
  }

  void CLogger::SetLine(T_LPCSTR ptr, GXINT nLine)
  {
    m_pMsg->SetCurrentTopLine(0);
    GXINT nCurLine = m_pMsg->LineFromPtr(ptr);
    m_pMsg->SetCurrentTopLine(nLine - nCurLine - 1);
  }

  void CLogger::PushFile(GXLPCWSTR szFilename, GXINT nTopLine, T_LPCSTR szCodes, size_t length)
  {
    m_pMsg->PushFile(szFilename, nTopLine);
    m_pMsg->GenerateCurLines((GXLPCSTR)szCodes, length);
  }

  void CLogger::PopFile()
  {
    m_pMsg->PopFile();
  }

  GXINT CLogger::GetLine(const TOKEN& token)
  {
    return m_pMsg->LineFromPtr(token.marker);
  }

  void CLogger::WriteMessageW(GXLPCWSTR szMessage)
  {
    m_pMsg->WriteMessageW(FALSE, szMessage);
  }

  GXUINT CLogger::MarkCode(GXUINT code, GXLPCSTR szMessage)
  {
    clStringW strMessage = szMessage;
    m_pMsg->UpdateErrorMessage(code & (~UVS_WARNING_MASK), strMessage);
    return code;
  }

  GXBOOL CLogger::HasError(int errcode) const
  {
    return (m_errorlist.find(errcode) != m_errorlist.end());
  }

  size_t CLogger::ErrorCount(GXBOOL bWithWarning) const
  {
    return bWithWarning ? m_nErrorCount : (m_nErrorCount - m_nWarningCount);
  }

  //////////////////////////////////////////////////////////////////////////
  
  RefString::RefString(CLLPCSTR szStableString)
    : m_pStr(szStableString)
    , m_nLength(clstd::strlenT(szStableString))
  {
  }

  RefString::RefString(CLLPCSTR pStablePtr, size_t length)
    : m_pStr(pStablePtr)
    , m_nLength(length)
  {
  }

  RefString::RefString()
    : m_pStr(NULL)
    , m_nLength(0)
  {
  }

  RefString::RefString(const clStringA& strStable)
    : m_pStr(strStable.CStr())
    , m_nLength(strStable.GetLength())
  {
  }

  RefString& RefString::Set(CLLPCSTR pStablePtr, size_t length)
  {
    m_pStr = pStablePtr;
    m_nLength = length;
    return *this;
  }

  int RefString::Compare(const RefString& rstr) const
  {
    const size_t n = m_nLength <= rstr.m_nLength ? m_nLength : rstr.m_nLength;
    int r = clstd::strncmpT(m_pStr, rstr.m_pStr, n);
    if(r != 0 || m_nLength == rstr.m_nLength) {
      return r;
    }
    else if(m_nLength > rstr.m_nLength) {
      return m_pStr[n];
    }
    return -(int)rstr.m_pStr[n];
  }

  b32 RefString::operator!=(const RefString& rstr) const
  {
    return Compare(rstr) != 0;
  }

  b32 RefString::operator<(const RefString& rstr) const
  {
    return Compare(rstr) < 0;
  }

  b32 RefString::operator<=(const RefString& rstr) const
  {
    return Compare(rstr) <= 0;
  }

  b32 RefString::operator>(const RefString& rstr) const
  {
    return Compare(rstr) > 0;
  }

  const ch& RefString::operator[](size_t index) const
  {
    ASSERT(m_pStr && index < m_nLength);
    return m_pStr[index];
  }

  clStringA& RefString::ToString(clStringA& str) const
  {
    str.Clear();
    return str.Append(m_pStr, m_nLength);
  }

  clStringW& RefString::ToString(clStringW& str) const
  {
    str.Clear();
    return str.Append(m_pStr, m_nLength);
  }

  CLLPCSTR RefString::GetPtr() const
  {
    return m_pStr;
  }

  size_t RefString::GetLength() const
  {
    return m_nLength;
  }

  b32 RefString::BeginsWith(CLLPCSTR szPrefix) const
  {
    size_t i = 0;
    while(szPrefix[i]) {
      if(i >= m_nLength || m_pStr[i] != szPrefix[i]) {
        return FALSE;
      }
      ++i;
    }
    return TRUE;
  }

  b32 RefString::EndsWith(CLLPCSTR szPostfix) const
  {
    if(szPostfix == NULL) {
      return TRUE;
    }
    const size_t nPostfixLen = clstd::strlenT(szPostfix);
    if(m_nLength < nPostfixLen) {
      return FALSE;
    }
    const size_t nTopIndex = m_nLength - nPostfixLen;

    size_t i = 0;
    while(szPostfix[i]) {
      if(i >= m_nLength || m_pStr[nTopIndex + i] != szPostfix[i]) {
        return FALSE;
      }
      ++i;
    }
    return TRUE;
  }

  b32 RefString::operator==(const RefString& rstr) const
  {
    return Compare(rstr) == 0;
  }

} // namespace UVShader