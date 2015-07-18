// Sample_UVShader.cpp : 定义控制台应用程序的入口点。
//

#include <tchar.h>
#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clPathFile.h>
#include <clStringSet.h>
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "TestExpressionParser.h"

void TestExpressionParser();

void TestFromFile(GXLPCSTR szFilename)
{
  clFile file;
  if(file.OpenExistingA(szFilename))
  {
    clBuffer* pBuffer = NULL;
    if(file.MapToBuffer(&pBuffer))
    {
      UVShader::ExpressionParser expp;
      const UVShader::ExpressionParser::SymbolArray* pSymbols;
      expp.Attach((const char*)pBuffer->GetPtr(), pBuffer->GetSize());

      expp.GenerateSymbols();
      pSymbols = expp.GetSymbolsArray();
      int nCount = 0;
      for(auto it = pSymbols->begin(); it != pSymbols->end(); ++it, ++nCount)
      {
        if(it->scope >= 0) {
          TRACE("<#%d:\"%s\"(%d)> ", nCount, it->sym.ToString(), it->scope);
        }
        else {
          TRACE("<#%d:\"%s\"> ", nCount, it->sym.ToString());
        }
      }

      TRACE("\ncount:%d(%f)\n", pSymbols->size(), (float)pBuffer->GetSize() / pSymbols->size());

      expp.Parse();
    }
    SAFE_DELETE(pBuffer);
  }
}

//////////////////////////////////////////////////////////////////////////

extern SAMPLE_EXPRESSION samplesNumeric[];
extern SAMPLE_EXPRESSION samplesOpercode[];
extern SAMPLE_EXPRESSION samplesExpression[];
extern SAMPLE_EXPRESSION samplesSimpleExpression[];

void TestExpressionParser(const SAMPLE_EXPRESSION* pSamples)
{
  //int k = 1, a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8;
  //int result = k*((a*b)+c+d*e);
  //int result = a+b+c+d*e/f;
  //int result = a?b:c?f:g;
  //int result = a>b?b:c;

  UVShader::ExpressionParser expp;
  for(int i = 0; pSamples[i].expression != 0; i++)
  {
    auto nSize = strlen(pSamples[i].expression);
    expp.Attach(pSamples[i].expression, nSize);
    expp.GenerateSymbols();
    auto pSymbols = expp.GetSymbolsArray();

    int nCount = 0;
    TRACE("%3d# ", i);
    for(auto it = pSymbols->begin(); it != pSymbols->end(); ++it)
    {
      TRACE("|%s|  ", it->sym.ToString());
      nCount++;
    }
    TRACE("(%d:%f)\n", nCount, (float)nSize / nCount);
    TRACE("%3d# \"%s\"\n", i, pSamples[i].expression);

    // 表达式解析
    UVShader::ExpressionParser::RTSCOPE scope = {0, pSymbols->size()};
    expp.ParseStatementAs_Expression(&scope, TRUE);

    // 检查操作堆栈
    if(pSamples[i].aOperStack)
    {
      int n = 0;
      for(auto it = expp.m_aDbgExpressionOperStack.begin();
        it != expp.m_aDbgExpressionOperStack.end(); ++it, ++n)
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

//////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
  clpathfile::LocalWorkingDirA("..");

  TestExpressionParser(samplesOpercode);
  TestExpressionParser(samplesNumeric);
  TestExpressionParser(samplesSimpleExpression);
  TestExpressionParser(samplesExpression);

  //TestFromFile("Test\\shaders\\ShaderToy\\Flame.txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt");
	return 0;
}

