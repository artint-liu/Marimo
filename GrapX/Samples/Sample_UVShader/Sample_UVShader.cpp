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

  // 使用继承类为了暴露ParseStatementAs_Expression接口进行测试
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

void TestShaderToys()
{
  clstd::FindFile find("Test\\shaders\\ShaderToy\\*.txt");
  clstd::FINDFILEDATAA find_data;
  while(find.GetFileA(&find_data))
  {
    clStringA strInput;
    clStringA strOutput = find_data.Filename;

    if(strOutput.EndsWith("[output].txt")) {
      continue;
    }

    clsize pos = clpathfile::FindExtensionA(strOutput);
    if(pos != clStringA::npos)
    {
      strOutput.Insert(pos, "[output]");
      clpathfile::CombinePathA(strInput, "Test\\shaders\\ShaderToy", find_data.Filename);
      clpathfile::CombinePathA(strOutput, "Test\\shaders\\ShaderToy", strOutput);
      TestFromFile(strInput, strOutput);
    }
  }
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

  //TestShaderToys();

  //TestFromFile("Test\\shaders\\ShaderToy\\Flame.txt", "Test\\shaders\\Flame[output].txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\Anatomy of an explosion.txt", "Test\\shaders\\Anatomy of an explosion[output].txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\Warp speed.txt", "Test\\shaders\\Warp speed[output].txt");
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt", "Test\\shaders\\TrivialRaytracer3[output].txt");
  
  //TestFromFile("Test\\shaders\\ShaderToy\\TrivialRaytracer3.txt");
	return 0;
}

