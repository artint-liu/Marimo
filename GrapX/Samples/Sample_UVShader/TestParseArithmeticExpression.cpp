#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "TestExpressionParser.h"

class TestExpression : public UVShader::CodeParser
{
public:
  GXBOOL TestParseExpression(STATEMENT* pStat, RTSCOPE* pScope)
  {
    m_aDbgExpressionOperStack.clear();
    STATEMENT stat = {StatementType_Expression};
    GXBOOL bret = ParseArithmeticExpression(pScope, &stat.expr.sRoot);
    if( ! bret)
    {
      TRACE("编译错误\n");
      m_aDbgExpressionOperStack.clear();
    }
    else if(TryGetNodeType(&stat.expr.sRoot) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX && stat.expr.sRoot.pNode) {
      IndexToPtr(stat.expr.sRoot.pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr(stat.expr.sRoot.pNode);
      DbgDumpSyntaxTree(stat.expr.sRoot.pNode, 0);
    }
    *pStat = stat;
    return bret;
    //return ParseStatementAs_Expression(pStat, pScope, TRUE);
  }
#ifdef ENABLE_GRAPH
  GXBOOL TestGraph(SYNTAXNODE::UN* pUnion)
  {
    //GXBOOL bret = Parse();
    UVShader::CodeParser::RTSCOPE scope(0, m_aTokens.size());
    GXBOOL bret = ParseArithmeticExpression(&scope, pUnion);

    if( ! bret) {
      return bret;
    }
    else if(TryGetNodeType(pUnion) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX && pUnion->pNode) {
      IndexToPtr(pUnion->pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr(pUnion->pNode);
      DbgDumpSyntaxTree(pUnion->pNode, 0);
    }
    return bret;
  }
#endif // #ifdef ENABLE_GRAPH
};

int MeasureStringsWidth(GXLPCSTR* aStrings)
{
  int i = 0;
  int nWidth = 0;
  while(aStrings[i]) {
    nWidth = clMax(nWidth, GXSTRLEN(aStrings[i++]));
  }
  return nWidth;
}

void TestExpressionParser(const SAMPLE_EXPRESSION* pSamples)
{
  //int k = 1, a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8;
  //int result = k*((a*b)+c+d*e);
  //int result = a+b+c+d*e/f;
  //int result = a?b:c?f:g;
  //int result = a>b?b:c;


  TestExpression expp;
  for(int i = 0; pSamples[i].expression != 0; i++)
  {
    auto nSize = strlen(pSamples[i].expression);
    expp.Attach(pSamples[i].expression, nSize);
    expp.GenerateTokens();
    auto pTokens = expp.GetTokensArray();

    int nCount = 0;
    //TRACE("%3d# ", i);
    TRACE("%d# \"%s\"\n", i, pSamples[i].expression);
    for(auto it = pTokens->begin(); it != pTokens->end(); ++it)
    {
      TRACE("[%d]\"%s\"\t(p=%d,s=%d,s2=%d)\n", it - pTokens->begin(), it->ToString(), it->precedence, it->scope, it->semi_scope);
      nCount++;
    }
    TRACE("(%d:%f)\n", nCount, (float)nSize / nCount);

    if(pSamples[i].bDbgBreak) {
      CLNOP; // 根据调试标记在这里下断点
    }

    // 表达式解析
    UVShader::CodeParser::RTSCOPE scope(0, pTokens->size());
    UVShader::CodeParser::STATEMENT stat;
    expp.TestParseExpression(&stat, &scope);

    clStringArrayA dbg_stack = expp.m_aDbgExpressionOperStack;

    // 检查操作堆栈
    if(pSamples[i].aOperStack)
    {

      int W = MeasureStringsWidth(pSamples[i].aOperStack);

      for(int n = 0; pSamples[i].aOperStack[n] != NULL; n++)
      {
        const GXBOOL bEuqal = dbg_stack[n] == pSamples[i].aOperStack[n];
        TRACE("%*s %s %s\n", W, pSamples[i].aOperStack[n], bEuqal ? "==" : "!=", dbg_stack[n]);
      }

      int n = 0;
      for(auto it = dbg_stack.begin();
        it != dbg_stack.end(); ++it, ++n)
      {
        if( ! pSamples[i].aOperStack[n]) {
          CLBREAK; // 样本堆栈肯定和后面计算堆栈长度一致
        }

        clStringA a = *it;
        clStringA b = pSamples[i].aOperStack[n];
        ASSERT(a == b);
      }
      ASSERT( ! pSamples[i].aOperStack[n]);
    }

    TRACE("\n\n");
    ASSERT(pSamples[i].expectation == 0 || nCount == pSamples[i].expectation);
  }
}
