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
    TRACE("--- ������ ---\n"); // ����ʱ���ɵ���
    GXBOOL bret = ParseArithmeticExpression(0, *pScope, &stat.sRoot);
    if( ! bret)
    {
      TRACE("�������\n");
      m_aDbgExpressionOperStack.clear();
    }
    else if(stat.sRoot.IsNode() && stat.sRoot.pNode) {
      TRACE("--- �ڴ��� ---\n"); // ����������ڴ����ṹ
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
    // ���Caps Lock On���������ʽ�����Լ�
    SHORT key_state = GetKeyState(VK_CAPITAL);
    if(TEST_FLAG(key_state, 1)) {
      return;
    }
    
    // ����׷�ٵ���������:
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


    // ���ʽ����
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
        // ����ṹ���ʼ����λ��, ��������ڷ�NULL�����ֻ�ܶ�ѡһ
        CLBREAK;
      }
    }

    const clStringArrayA& dbg_stack = expp.DbgGetExpressionStack();

    // ��������ջ
    if(pSamples[i].aOperStack)
    {
      TRACE("--- �Ա��﷨�б�(�ο�����:�������) ---\n");
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
      TRACE("--- ����﷨�б�(û�вο��б�) ---\n");
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
    // ��ʽ����
    {"- 666", VALUE::State_OK},
    {"+\t666", VALUE::State_OK},
    {"- \t\r666", VALUE::State_OK},
    {"- \t\r6f66", VALUE::State_IllegalChar},
    {"- \t\r", VALUE::State_SyntaxError},
    {"- \t\r\\97", VALUE::State_IllegalChar},

    // ��������
    {"123u", VALUE::State_OK},
    {"123456", VALUE::State_OK},
    {"-123456", VALUE::State_OK},
    {"-9223372036854775808", VALUE::State_OK}, // �����Ҫ��, ��׼�ο�, 64λ�з�����Сֵ
    { "9223372036854775807", VALUE::State_OK}, // �����Ҫ��, ��׼�ο�, 64λ�з������ֵ
    {"18446744073709551615", VALUE::State_OK}, // �����Ҫ��, ��׼�ο�, 64λ�޷������ֵ

    {"-9223372036854775807", VALUE::State_OK}, // �ٽ缫Сֵ
    {"-9223372036854775806", VALUE::State_OK}, // �ٽ缫Сֵ
    { "9223372036854775805", VALUE::State_OK}, // �ٽ�32λ�з��ż���ֵ
    { "9223372036854775806", VALUE::State_OK}, // �ٽ�32λ�з��ż���ֵ
    {"18446744073709551612", VALUE::State_OK}, // �ٽ�64λ�޷��ż�Сֵ
    {"18446744073709551614", VALUE::State_OK}, // �ٽ�64λ�޷��ż�Сֵ
    {"18446744073709551613", VALUE::State_OK}, // �ٽ�64λ�޷��ż�Сֵ

    { "9223372036854775808", VALUE::State_OK}, // �з���64λ���ֵ+1���ᱻ����Ϊ64λ�޷���

    {"-9223372036854775809", VALUE::State_Overflow},
    {"18446744073709551616", VALUE::State_Overflow},
    {"184467440737095516140", VALUE::State_Overflow},

    // �����ڶ�λ����
    {"18446744073709551605", VALUE::State_OK},
    {"18446744073709551625", VALUE::State_Overflow},
    { "9223372036854775817", VALUE::State_OK}, // ����Ϊ64λ�޷�����
    { "9223372036854775877", VALUE::State_OK}, // ����Ϊ64λ�޷�����
    {"-9223372036854775818", VALUE::State_Overflow},


    
    // ����
    {"1e18", VALUE::State_OK},
    {"1e19", VALUE::State_OK},
    {"2e19", VALUE::State_OK},
    {"123e4", VALUE::State_OK},
    {"-123e4", VALUE::State_OK},

    // ����
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

    {"2.1583468527305348600363230610125", VALUE::State_OK}, // ����û��������������ֱ�ʡ�Ե�

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
      // ȥ������ǰ׺
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
        TRACE("ERROR : �����ַ���û�а������ָ�ʽ��չ:%s\n", samp[i].expression);
        CLBREAK;
      }
      else if(_CL_NOT_((*pArray)[0].IsNumeric()))
      {
        TRACE("%s(%d)\n", samp[i].szSourceFile, samp[i].nLine);
        TRACE("ERROR : ��չ��û�н���Ϊ����:%s\n", samp[i].expression);
        CLBREAK;
      }
    }
  }
}