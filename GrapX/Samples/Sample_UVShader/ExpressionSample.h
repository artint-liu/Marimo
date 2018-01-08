#ifndef _TEST_EXPRESSION_PARSER_H_
#define _TEST_EXPRESSION_PARSER_H_

struct SAMPLE_EXPRESSION
{
  int         id;
  GXLPCSTR    szSourceFile;
  GXINT       nLine;
  const char* expression;
  int         expectation;  // 预期的token数量, 0表示任意
  GXLPCSTR*   aOperStack;   // 预期的语法结构
  GXLPCSTR    szResult;     // 编译结果, "FAILED"表示测试表达式编译会失败, "OK"表示成功, 不关心填NULL
  GXBOOL      bDbgBreak;    // 调试中断
};

#endif // _TEST_EXPRESSION_PARSER_H_