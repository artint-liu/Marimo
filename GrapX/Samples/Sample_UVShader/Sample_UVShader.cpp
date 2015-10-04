// Sample_UVShader.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "TestExpressionParser.h"

void TestExpressionParser();
void TestFromFile(GXLPCSTR szFilename, GXLPCSTR szOutput);


//////////////////////////////////////////////////////////////////////////

extern SAMPLE_EXPRESSION samplesNumeric[];
extern SAMPLE_EXPRESSION samplesOpercode[];
extern SAMPLE_EXPRESSION samplesExpression[];
extern SAMPLE_EXPRESSION samplesIfExpression[];
extern SAMPLE_EXPRESSION samplesForExpression[];
extern SAMPLE_EXPRESSION samplesSimpleExpression[];

void TestExpressionParser(const SAMPLE_EXPRESSION* pSamples)
{
  //int k = 1, a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8;
  //int result = k*((a*b)+c+d*e);
  //int result = a+b+c+d*e/f;
  //int result = a?b:c?f:g;
  //int result = a>b?b:c;

  // ʹ�ü̳���Ϊ�˱�¶ParseStatementAs_Expression�ӿڽ��в���
  class TestExpression : public UVShader::CodeParser
  {
  public:
    GXBOOL TestParseExpression(STATEMENT* pStat, RTSCOPE* pScope)
    {
      return ParseStatementAs_Expression(pStat, pScope, TRUE);
    }
  };

  TestExpression expp;
  for(int i = 0; pSamples[i].expression != 0; i++)
  {
    auto nSize = strlen(pSamples[i].expression);
    expp.Attach(pSamples[i].expression, nSize);
    expp.GenerateSymbols();
    auto pSymbols = expp.GetSymbolsArray();

    int nCount = 0;
    //TRACE("%3d# ", i);
    TRACE("%d# \"%s\"\n", i, pSamples[i].expression);
    for(auto it = pSymbols->begin(); it != pSymbols->end(); ++it)
    {
      TRACE("[%d]\"%s\"\t(p=%d,s=%d,s2=%d)\n", it - pSymbols->begin(), it->ToString(), it->precedence, it->scope, it->semi_scope);
      nCount++;
    }
    TRACE("(%d:%f)\n", nCount, (float)nSize / nCount);

    if(pSamples[i].bDbgBreak) {
      CLNOP; // ���ݵ��Ա���������¶ϵ�
    }

    // ���ʽ����
    UVShader::CodeParser::RTSCOPE scope(0, pSymbols->size());
    UVShader::CodeParser::STATEMENT stat;
    expp.TestParseExpression(&stat, &scope);

    // ��������ջ
    if(pSamples[i].aOperStack)
    {
      int n = 0;
      for(auto it = expp.m_aDbgExpressionOperStack.begin();
        it != expp.m_aDbgExpressionOperStack.end(); ++it, ++n)
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

    TRACE("\n\n");
    ASSERT(pSamples[i].expectation == 0 || nCount == pSamples[i].expectation);
  }
}

//////////////////////////////////////////////////////////////////////////

void TestFlowIf()
{
  int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5;

  if(a < b)
    if(b < c)
      if(c > d)
        a = a;
      else if(d < e)
        b = b;
      else
        c = c;


  switch(a)
  {
  case 0:
  case 1:
    if(b>c)
    {
  case 2:
    break;
    }
  default:
    break;
  };

  do 
  {
    a++;
  } while (a < b);
}

int _tmain(int argc, _TCHAR* argv[])
{
  int a = 1, b = 2, c = 3;
  TestFlowIf();
  clpathfile::LocalWorkingDirA("..");

  //TestExpressionParser(samplesOpercode);
  //TestExpressionParser(samplesNumeric);
  //TestExpressionParser(samplesSimpleExpression);
  //TestExpressionParser(samplesIfExpression);
  //TestExpressionParser(samplesForExpression);
  //TestExpressionParser(samplesExpression);

  TestFromFile("Test\\shaders\\std_samples.uvs", "Test\\shaders\\std_samples[output].txt");
  TestFromFile("Test\\shaders\\ShaderToy\\Flame.txt", "Test\\shaders\\Flame_output.txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt");
	return 0;
}

