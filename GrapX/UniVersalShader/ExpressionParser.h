#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

class ExpressionParser : public SmartStreamA
{
private:
  static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);
  static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);
public:
  ExpressionParser();
  b32 Attach(const char* szExpression, clsize nSize);
};

#endif // #ifndef _EXPRESSION_PARSER_H_