#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ArithmeticExpression.h"
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "ExpressionSample.h"

class TestExpression : public UVShader::CodeParser
{
public:
  clStringArrayA m_aCommand;

  GXBOOL TestParseExpression(STATEMENT* pStat, RTSCOPE* pScope)
  {
    m_aDbgExpressionOperStack.clear();
    m_aCommand.clear();
    STATEMENT stat = {StatementType_Expression};
    GXBOOL bret = ParseArithmeticExpression(*pScope, &stat.expr.sRoot);
    if( ! bret)
    {
      TRACE("�������\n");
      m_aDbgExpressionOperStack.clear();
    }
    else if(TryGetNodeType(&stat.expr.sRoot) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX && stat.expr.sRoot.pNode) {
      IndexToPtr(stat.expr.sRoot.pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr(stat.expr.sRoot.pNode);
      TRACE("---\n");
      DbgDumpSyntaxTree(&m_aCommand, stat.expr.sRoot.pNode, 0);
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

int MeasureStringsWidth(const clStringArrayA& aStrings)
{
  int nWidth = 0;
  for(auto it = aStrings.begin(); it != aStrings.end(); ++it) {
    nWidth = clMax(nWidth, (int)it->GetLength());
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
    expp.Attach(pSamples[i].expression, nSize, 0, NULL);
    expp.GenerateTokens();
    auto pTokens = expp.GetTokensArray();

    int nCount = 0;
    //TRACE("%3d# ", i);
    TRACE("%s(%d)\n", pSamples[i].szSourceFile, pSamples[i].nLine);
    TRACE("%d# \"%s\"\n", i, pSamples[i].expression);
    for(auto it = pTokens->begin(); it != pTokens->end(); ++it)
    {
      TRACE("[%d]\"%s\"\t(p=%d,s=%d,s2=%d)\n", it - pTokens->begin(), it->ToString(), it->precedence, it->scope, it->semi_scope);
      nCount++;
    }
    TRACE("(%d:%f)\n", nCount, (float)nSize / nCount);

    if(pSamples[i].bDbgBreak) {
      CLNOP; // ���ݵ��Ա���������¶ϵ�
    }

    // ���ʽ����
    UVShader::CodeParser::RTSCOPE scope(0, pTokens->size());
    UVShader::CodeParser::STATEMENT stat;
    expp.TestParseExpression(&stat, &scope);

    const clStringArrayA& dbg_stack = expp.DbgGetExpressionStack();

    // ��������ջ
    if(pSamples[i].aOperStack)
    {

      int W0 = MeasureStringsWidth(pSamples[i].aOperStack);
      int W1 = MeasureStringsWidth(dbg_stack);

      for(int n = 0; pSamples[i].aOperStack[n] != NULL; n++)
      {
        const GXBOOL bEuqal = dbg_stack[n] == pSamples[i].aOperStack[n];
        TRACE("%*s %s %*s | %s\n", W0, pSamples[i].aOperStack[n], bEuqal ? "==" : "!=", -W1, dbg_stack[n], expp.m_aCommand[n]);
      }

      int n = 0;
      for(auto it = dbg_stack.begin();
        it != dbg_stack.end(); ++it, ++n)
      {
        if( ! pSamples[i].aOperStack[n]) {
          CLBREAK; // ������ջ�϶��ͺ�������ջ����һ��
        }

        clStringA a = *it;
        clStringA b = pSamples[i].aOperStack[n];
        ASSERT(a == b);
      }
      ASSERT( ! pSamples[i].aOperStack[n]);
    }
    else {
      for(auto it = expp.m_aCommand.begin(); it != expp.m_aCommand.end(); ++it)
      {
        TRACE("%s\n", *it);
      }
    }

    TRACE("\n\n");
    ASSERT(pSamples[i].expectation == 0 || nCount == pSamples[i].expectation);
  }
}
