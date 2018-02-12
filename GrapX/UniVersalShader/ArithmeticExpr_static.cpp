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
  //ArithmeticExpression::INTRINSIC_TYPE ArithmeticExpression::s_aIntrinsicType[] = {
  //  {"int",   3, 4, 4},
  //  {"vec",   3, 4, 0},
  //  {"bool",  4, 4, 0},
  //  {"half",  4, 4, 4},
  //  {"uint",  4, 4, 4},
  //  {"dword", 5, 4, 4},
  //  {"float", 5, 4, 4},
  //  {"double",6, 4, 4},
  //  {NULL},
  //};

  //////////////////////////////////////////////////////////////////////////
  MEMBERLIST s_aIntXYZW[] = {
    {"int", "x"},
    {"int", "r"},
    {"int", "y"},
    {"int", "g"},
    {"int", "z"},
    {"int", "b"},
    {"int", "w"},
    {"int", "a"},
  };

  MEMBERLIST s_aVecXYZW[] = {
    {"float", "x"},
    {"float", "r"},
    {"float", "y"},
    {"float", "g"},
    {"float", "z"},
    {"float", "b"},
    {"float", "w"},
    {"float", "a"},
  };

  MEMBERLIST s_aHalfXYZW[] = {
    {"half", "x"},
    {"half", "r"},
    {"half", "y"},
    {"half", "g"},
    {"half", "z"},
    {"half", "b"},
    {"half", "w"},
    {"half", "a"},
  };

  MEMBERLIST s_aUintXYZW[] = {
    {"uint", "x"},
    {"uint", "r"},
    {"uint", "y"},
    {"uint", "g"},
    {"uint", "z"},
    {"uint", "b"},
    {"uint", "w"},
    {"uint", "a"},
  };

  MEMBERLIST s_aDoubleXYZW[] = {
    {"double", "x"},
    {"double", "r"},
    {"double", "y"},
    {"double", "g"},
    {"double", "z"},
    {"double", "b"},
    {"double", "w"},
    {"double", "a"},
  };

  STRUCTDESC s_aIntrinsicStruct[] =
  {
    {"int2", s_aIntXYZW, 4, "int", "xy", "rg"},
    {"int3", s_aIntXYZW, 6, "int", "xyz", "rgb"},
    {"int4", s_aIntXYZW, 8, "int", "xyzw", "rgba"},
    
    {"vec2", s_aVecXYZW, 4, "float", "xy", "rg"},
    {"vec3", s_aVecXYZW, 6, "float", "xyz", "rgb"},
    {"vec4", s_aVecXYZW, 8, "float", "xyzw", "rgba"},
    
    {"half2", s_aHalfXYZW, 4, "half", "xy", "rg"},
    {"half3", s_aHalfXYZW, 6, "half", "xyz", "rgb"},
    {"half4", s_aHalfXYZW, 8, "half", "xyzw", "rgba"},

    {"uint2", s_aUintXYZW, 4, "uint", "xy", "rg"},
    {"uint3", s_aUintXYZW, 6, "uint", "xyz", "rgb"},
    {"uint4", s_aUintXYZW, 8, "uint", "xyzw", "rgba"},

    {"float2", s_aVecXYZW, 4, "float", "xy", "rg"},
    {"float3", s_aVecXYZW, 6, "float", "xyz", "rgb"},
    {"float4", s_aVecXYZW, 8, "float", "xyzw", "rgba"},
    
    {"double2", s_aDoubleXYZW, 4, "double", "xy", "rg"},
    {"double3", s_aDoubleXYZW, 6, "double", "xyz", "rgb"},
    {"double4", s_aDoubleXYZW, 8, "double", "xyzw", "rgba"},

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
    {'(', ')', FALSE},   // 圆括号
    {'[', ']', FALSE},   // 方括号
    {'{', '}', FALSE},   // 花括号
    {'?', ':', TRUE},    // 冒号不匹配会被解释为语意
  };

} // namespace UVShader


