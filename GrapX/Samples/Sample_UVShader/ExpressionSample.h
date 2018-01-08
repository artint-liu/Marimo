#ifndef _TEST_EXPRESSION_PARSER_H_
#define _TEST_EXPRESSION_PARSER_H_

struct SAMPLE_EXPRESSION
{
  int         id;
  GXLPCSTR    szSourceFile;
  GXINT       nLine;
  const char* expression;
  int         expectation;  // Ԥ�ڵ�token����, 0��ʾ����
  GXLPCSTR*   aOperStack;   // Ԥ�ڵ��﷨�ṹ
  GXLPCSTR    szResult;     // ������, "FAILED"��ʾ���Ա��ʽ�����ʧ��, "OK"��ʾ�ɹ�, ��������NULL
  GXBOOL      bDbgBreak;    // �����ж�
};

#endif // _TEST_EXPRESSION_PARSER_H_