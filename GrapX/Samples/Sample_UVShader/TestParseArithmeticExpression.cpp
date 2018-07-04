#include <Marimo.H>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include <clStablePool.h>
#include "../../../GrapX/UniVersalShader/ArithmeticExpression.h"
#include "../../../GrapX/UniVersalShader/ExpressionParser.h"
#include "ExpressionSample.h"

extern SAMPLE_EXPRESSION samplesNumeric[];

class TestExpression : public UVShader::CodeParser
{
public:
  clStringArrayA m_aCommand;

  TestExpression() : UVShader::CodeParser(NULL, NULL)
  {
  }

  GXBOOL TestParseExpression(STATEMENT* pStat, TKSCOPE* pScope)
  {
    m_aDbgExpressionOperStack.clear();
    m_aCommand.clear();
    STATEMENT stat = {StatementType_Empty};
    TRACE("--- 解析树 ---\n"); // 解析时生成的树
    GXBOOL bret = ParseArithmeticExpression(0, *pScope, &stat.sRoot);
    if( ! bret)
    {
      TRACE("编译错误\n");
      m_aDbgExpressionOperStack.clear();
    }
    else if(stat.sRoot.IsNode() && stat.sRoot.pNode) {
      TRACE("--- 内存树 ---\n"); // 重新输出的内存树结构
      DbgDumpSyntaxTree(&m_aCommand, stat.sRoot.pNode, 0);
    }
    *pStat = stat;
    return bret;
  }
#ifdef ENABLE_GRAPH
  GXBOOL TestGraph(SYNTAXNODE::GLOB* pUnion)
  {
    //GXBOOL bret = Parse();
    UVShader::CodeParser::TKSCOPE scope(0, m_aTokens.size());
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
    nWidth = clMax(nWidth, (int)GXSTRLEN(aStrings[i++]));
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
    // 如果Caps Lock On就跳过表达式解析自检
    SHORT key_state = GetKeyState(VK_CAPITAL);
    if(TEST_FLAG(key_state, 1)) {
      return;
    }
    
    // 可以追踪的两个变量:
    // pSamples[i].nLine
    // pSamples[i].id

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


    // 表达式解析
    UVShader::CodeParser::TKSCOPE scope(0, pTokens->size());
    UVShader::CodeParser::STATEMENT stat;
    GXBOOL bCompilation = expp.TestParseExpression(&stat, &scope);
    if(pSamples[i].szResult != NULL)
    {
      if(GXSTRCMP(pSamples[i].szResult, "FAILED") == 0)
      {
        ASSERT(_CL_NOT_(bCompilation));
      }
      else if(GXSTRCMP(pSamples[i].szResult, "OK") == 0)
      {
        ASSERT(bCompilation);
      }
      else
      {
        // 避免结构体初始化串位置, 所以这个在非NULL情况下只能二选一
        CLBREAK;
      }
    }

    const clStringArrayA& dbg_stack = expp.DbgGetExpressionStack();

    // 检查操作堆栈
    if(pSamples[i].aOperStack)
    {
      TRACE("--- 对比语法列表(参考样本:解析结果) ---\n");
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
          CLBREAK; // 样本堆栈肯定和后面计算堆栈长度一致
        }

        clStringA a = *it;
        clStringA b = pSamples[i].aOperStack[n];
        ASSERT(a == b);
      }
      ASSERT( ! pSamples[i].aOperStack[n]);
    }
    else {
      TRACE("--- 输出语法列表(没有参考列表) ---\n");
      for(auto it = expp.m_aCommand.begin(); it != expp.m_aCommand.end(); ++it)
      {
        TRACE("%s\n", *it);
      }
    }

    TRACE("\n\n");
    ASSERT(pSamples[i].expectation == 0 || nCount == pSamples[i].expectation);
  }
}

//////////////////////////////////////////////////////////////////////////

namespace DigitalParsing
{
  using namespace UVShader;
  //typedef ArithmeticExpression::VALUE VALUE;
  typedef VALUE::State State;
  //typedef TOKEN TOKEN;
  struct SAMPLE
  {
    GXLPCSTR  str;
    State     state;
  };

  static SAMPLE aSampleDigitals[] = {
    // 格式测试
    {"- 666", VALUE::State_OK},
    {"+\t666", VALUE::State_OK},
    {"- \t\r666", VALUE::State_OK},
    {"- \t\r6f66", VALUE::State_IllegalChar},
    {"- \t\r", VALUE::State_SyntaxError},
    {"- \t\r\\97", VALUE::State_IllegalChar},

    // 整数类型
    {"123u", VALUE::State_OK},
    {"123456", VALUE::State_OK},
    {"-123456", VALUE::State_OK},
    {"-9223372036854775808", VALUE::State_OK}, // 这个不要改, 基准参考, 64位有符号最小值
    { "9223372036854775807", VALUE::State_OK}, // 这个不要改, 基准参考, 64位有符号最大值
    {"18446744073709551615", VALUE::State_OK}, // 这个不要改, 基准参考, 64位无符号最大值

    {"-9223372036854775807", VALUE::State_OK}, // 临界极小值
    {"-9223372036854775806", VALUE::State_OK}, // 临界极小值
    { "9223372036854775805", VALUE::State_OK}, // 临界32位有符号极大值
    { "9223372036854775806", VALUE::State_OK}, // 临界32位有符号极大值
    {"18446744073709551612", VALUE::State_OK}, // 临界64位无符号极小值
    {"18446744073709551614", VALUE::State_OK}, // 临界64位无符号极小值
    {"18446744073709551613", VALUE::State_OK}, // 临界64位无符号极小值

    { "9223372036854775808", VALUE::State_OK}, // 有符号64位最大值+1，会被解析为64位无符号

    {"-9223372036854775809", VALUE::State_Overflow},
    {"18446744073709551616", VALUE::State_Overflow},
    {"184467440737095516140", VALUE::State_Overflow},

    // 倒数第二位测试
    {"18446744073709551605", VALUE::State_OK},
    {"18446744073709551625", VALUE::State_Overflow},
    { "9223372036854775817", VALUE::State_OK}, // 解析为64位无符号数
    { "9223372036854775877", VALUE::State_OK}, // 解析为64位无符号数
    {"-9223372036854775818", VALUE::State_Overflow},


    
    // 浮点
    {"1e18", VALUE::State_OK},
    {"1e19", VALUE::State_OK},
    {"2e19", VALUE::State_OK},
    {"123e4", VALUE::State_OK},
    {"-123e4", VALUE::State_OK},

    // 浮点
    {"123.456", VALUE::State_OK},
    {"0.12345", VALUE::State_OK},
    {".123456", VALUE::State_OK},
    {"123456.", VALUE::State_OK},
    {"-123.456", VALUE::State_OK},
    {"-0.12345", VALUE::State_OK},
    {"-.123456", VALUE::State_OK},
    {"-123456.", VALUE::State_OK},
    {"12.34.56", VALUE::State_SyntaxError},
    {"-12.34.56", VALUE::State_SyntaxError},

    {"123e3", VALUE::State_OK},
    {"1.23e3", VALUE::State_OK},
    {"123e-3", VALUE::State_OK},
    {"1.23e-3", VALUE::State_OK},
    {"-123e3", VALUE::State_OK},
    {"-1.23e3", VALUE::State_OK},
    {"-123e-3", VALUE::State_OK},
    {"-1.23e-3", VALUE::State_OK},

    {"2.1583468527305348600363230610125", VALUE::State_OK}, // 浮点没有溢出，超出部分被省略掉

    {NULL, },
  };

  void Test1()
  {
    for(int i = 0; aSampleDigitals[i].str != NULL; i++)
    {
      TOKEN token;
      VALUE value;
      token.marker = aSampleDigitals[i].str;
      token.length = GXSTRLEN(aSampleDigitals[i].str);

      VALUE v;
      State state = v.set(token);
      clStringA str;
      if(v.rank == VALUE::Rank_Unsigned64)
      {
        str.Format("\"%s\" => %llu(Unsigned 64)\t\t%d|%d", aSampleDigitals[i].str, v.uValue64, aSampleDigitals[i].state, state);
      }
      else if(v.rank == VALUE::Rank_Signed64)
      {
        str.Format("\"%s\" => %lld(Signed 64)\t\t%d|%d", aSampleDigitals[i].str, v.nValue64, aSampleDigitals[i].state, state);
      }
      else if(v.rank == VALUE::Rank_Double)
      {
        str.Format("\"%s\" => %f(double)\t\t%d|%d", aSampleDigitals[i].str, v.fValue64, aSampleDigitals[i].state, state);
      }
      else if(state != VALUE::State_OK)
      {
        str.Format("\"%s\" => XXX(failed)\t\t%d|%d", aSampleDigitals[i].str, aSampleDigitals[i].state, state);
      }

      ASSERT(aSampleDigitals[i].state == state);

      TRACE(str.AppendFormat("%s\n", aSampleDigitals[i].state == state ? "" : " (X)"));
    }
  }

  void Test2()
  {
    UVShader::CodeParser expp(NULL, NULL);

    SAMPLE_EXPRESSION* samp = samplesNumeric;

    for(int i = 0; samp[i].expression != NULL; i++)
    {
      // 去掉符号前缀
      const ch* pstr = samp[i].expression;
      if(pstr[0] == '+' || pstr[0] == '-')
      {
        pstr++;
      }
      clsize len = clstd::strlenT(pstr);

      expp.Attach(pstr, len, 0, 0);
      expp.GenerateTokens();
      const UVShader::TOKEN::Array* pArray = expp.GetTokensArray();

      if(pArray->size() != 1) {
        TRACE("%s(%d)\n", samp[i].szSourceFile, samp[i].nLine);
        TRACE("ERROR : 样本字符串没有按照数字格式扩展:%s\n", samp[i].expression);
        CLBREAK;
      }
      else if(_CL_NOT_((*pArray)[0].IsNumeric()))
      {
        TRACE("%s(%d)\n", samp[i].szSourceFile, samp[i].nLine);
        TRACE("ERROR : 扩展的没有解析为数字:%s\n", samp[i].expression);
        CLBREAK;
      }
    }
  }
}