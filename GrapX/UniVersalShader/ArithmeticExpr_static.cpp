#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include <clStablePool.h>
#include "ArithmeticExpression.h"
#include "ExpressionParser.h"

//#include "clTextLines.h"
//#include "../User/DataPoolErrorMsg.h"

namespace UVShader
{
  //static const int c_plus_minus_precedence = 12; // +, - 作为符号时的优先级

  // 这个按照ASCII顺序分布, "+",",","-" 分别是43，44，45
  ArithmeticExpression::MBO ArithmeticExpression::s_plus_minus[] = {
    {1, "+", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 正号
    {}, // 占位
    {1, "-", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 负号
  };

  ArithmeticExpression::MBO ArithmeticExpression::s_semantic =  // ":" 作为语意的优先级
    {1, ":", OPP(13) };

  ArithmeticExpression::MBO ArithmeticExpression::s_Operator1[] = {
    {1, ".", OPP(13)},
    //{1, "+", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 正号
    //{1, "−", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 负号
    {1, "!", OPP(12), TRUE, UNARY_RIGHT_OPERAND},
    {1, "~", OPP(12), TRUE, UNARY_RIGHT_OPERAND},
    //{1, "&", OPP(12), TRUE, UNARY_RIGHT_OPERAND},
    {1, "*", OPP(11)},
    {1, "/", OPP(11)},
    {1, "%", OPP(11)},
    {1, "+", OPP(10)}, // 加法
    {1, "-", OPP(10)}, // 减法
    {1, "<", OPP( 8)},
    {1, ">", OPP( 8)},
    {1, "&", OPP( 6)},
    {1, "^", OPP( 5), TRUE, UNARY_RIGHT_OPERAND},
    {1, "|", OPP( 4)},
    {1, "=", OPP( 1)},
    {1, "?", OPP( 1)}, // ?: 操作符
    {1, ":", OPP( 1)}, // ?: 操作符
    {1, ",", OPP( 0)},
    {NULL,},
  };

  ArithmeticExpression::MBO ArithmeticExpression::s_Operator2[] = {
    {2, "--", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 默认 --n
    {2, "++", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // ++n
    {2, ">>", OPP( 9)},
    {2, "<<", OPP( 9)},
    {2, "<=", OPP( 8)},
    {2, ">=", OPP( 8)},
    {2, "==", OPP( 7)},
    {2, "!=", OPP( 7)},
    {2, "&&", OPP( 3)},
    {2, "||", OPP( 2)},
    {2, "+=", OPP( 1)},
    {2, "-=", OPP( 1)},
    {2, "*=", OPP( 1)},
    {2, "/=", OPP( 1)},
    {2, "%=", OPP( 1)},
    {2, "&=", OPP( 1)},
    {2, "^=", OPP( 1)},
    {2, "|=", OPP( 1)},
    {NULL,},

    // 不会用到的符号
    {2, "::",  OPP(-1)},
    {2, "->",  OPP(-1)},
    {2, ".*",  OPP(-1)},
    {2, "->*", OPP(-1)}, 
  };

  // 多重解释的符号
  ArithmeticExpression::MBO ArithmeticExpression::s_UnaryLeftOperand[] = {
    {2, "++", OPP(13), TRUE, UNARY_LEFT_OPERAND},  // n++
    {},                                            // 占位
    {2, "--", OPP(13), TRUE, UNARY_LEFT_OPERAND},  // n--
    {NULL,},
  };

  ArithmeticExpression::MBO ArithmeticExpression::s_Operator3[] = {
    {3, "<<=", OPP(1)},
    {3, ">>=", OPP(1)},
    {NULL,},
  };

  // 全是数字类型
  ArithmeticExpression::INTRINSIC_TYPE ArithmeticExpression::s_aIntrinsicType[] = {
    {"int",   3, 4, 4},
    {"vec",   3, 4, 0},
    {"bool",  4, 4, 0},
    {"half",  4, 4, 4},
    {"uint",  4, 4, 4},
    {"dword", 5, 4, 4},
    {"float", 5, 4, 4},
    {"double",6, 4, 4},
    {NULL},
  };

  //////////////////////////////////////////////////////////////////////////
  MEMBERLIST s_aIntXYZW[] = {
    {"int", "x"},
    {"int", "y"},
    {"int", "z"},
    {"int", "w"},
  };

  MEMBERLIST s_aVecXYZW[] = {
    {"float", "x"},
    {"float", "y"},
    {"float", "z"},
    {"float", "w"},
  };

  MEMBERLIST s_aHalfXYZW[] = {
    {"half", "x"},
    {"half", "y"},
    {"half", "z"},
    {"half", "w"},
  };

  MEMBERLIST s_aUintXYZW[] = {
    {"uint", "x"},
    {"uint", "y"},
    {"uint", "z"},
    {"uint", "w"},
  };

  MEMBERLIST s_aDoubleXYZW[] = {
    {"double", "x"},
    {"double", "y"},
    {"double", "z"},
    {"double", "w"},
  };

  STRUCTDESC s_aIntrinsicStruct[] =
  {
    {"int2", s_aIntXYZW, 2},
    {"int3", s_aIntXYZW, 3},
    {"int4", s_aIntXYZW, 4},
    
    {"vec2", s_aVecXYZW, 2},
    {"vec3", s_aVecXYZW, 3},
    {"vec4", s_aVecXYZW, 4},
    
    {"half2", s_aHalfXYZW, 2},
    {"half3", s_aHalfXYZW, 3},
    {"half4", s_aHalfXYZW, 4},

    {"uint2", s_aUintXYZW, 2},
    {"uint3", s_aUintXYZW, 3},
    {"uint4", s_aUintXYZW, 4},

    {"float2", s_aVecXYZW, 2},
    {"float3", s_aVecXYZW, 3},
    {"float4", s_aVecXYZW, 4},
    
    {"double2", s_aDoubleXYZW, 2},
    {"double3", s_aDoubleXYZW, 3},
    {"double4", s_aDoubleXYZW, 4},

    {NULL},
  };

  STRUCTDESC s_aBaseType[] =
  {
    {"float"},
    {"half"},
    {"int"},
    {"bool"},
    {"uint"},
    {"double"},
    {NULL}
  };


/*
[] [float] [x]
[] [float] [y]
[] [float] [z]
[] [float] [w]
[] [float w] []
[] [float z] [float w]
[] [float y] [float z;float w]
[] [float x] [float y;float z;float w]
[] [float x;float y;float z;float w] [;]
{float x;float y;float z;float w}
*/

  ArithmeticExpression::PAIRMARK ArithmeticExpression::s_PairMark[] = {
    {'(', ')', FALSE, FALSE},   // 圆括号
    {'[', ']', FALSE, FALSE},   // 方括号
    {'{', '}', TRUE , FALSE},   // 花括号
    {'?', ':', FALSE, TRUE},    // 冒号不匹配会被解释为语意
  };

} // namespace UVShader


