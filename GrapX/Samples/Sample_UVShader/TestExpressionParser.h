#ifndef _TEST_EXPRESSION_PARSER_H_
#define _TEST_EXPRESSION_PARSER_H_

struct SAMPLE_EXPRESSION
{
  int         id;
  const char* expression;
  int         expectation;
  GXLPCSTR*   aOperStack;
};

#endif // _TEST_EXPRESSION_PARSER_H_