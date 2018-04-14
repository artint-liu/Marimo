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
  GXLPCSTR STR_VOID   = "void";

  GXLPCSTR STR_INT    = "int";
  GXLPCSTR STR_UINT   = "uint";
  GXLPCSTR STR_HALF   = "half";
  GXLPCSTR STR_BOOL   = "bool";
  GXLPCSTR STR_FLOAT  = "float";
  GXLPCSTR STR_FLOAT2 = "float2";
  GXLPCSTR STR_FLOAT3 = "float3";
  GXLPCSTR STR_FLOAT4 = "float4";

  GXLPCSTR STR_FLOAT2x2 = "float2x2";
  GXLPCSTR STR_FLOAT2x3 = "float2x3";
  GXLPCSTR STR_FLOAT2x4 = "float2x4";
  GXLPCSTR STR_FLOAT3x2 = "float3x2";
  GXLPCSTR STR_FLOAT3x3 = "float3x3";
  GXLPCSTR STR_FLOAT3x4 = "float3x4";
  GXLPCSTR STR_FLOAT4x2 = "float4x2";
  GXLPCSTR STR_FLOAT4x3 = "float4x3";
  GXLPCSTR STR_FLOAT4x4 = "float4x4";

  GXLPCSTR STR_DOUBLE  = "double";
  GXLPCSTR STR_DOUBLE2 = "double2";
  GXLPCSTR STR_DOUBLE3 = "double3";
  GXLPCSTR STR_DOUBLE4 = "double4";

  GXLPCSTR STR_DOUBLE2x2 = "double2x2";
  GXLPCSTR STR_DOUBLE2x3 = "double2x3";
  GXLPCSTR STR_DOUBLE2x4 = "double2x4";
  GXLPCSTR STR_DOUBLE3x2 = "double3x2";
  GXLPCSTR STR_DOUBLE3x3 = "double3x3";
  GXLPCSTR STR_DOUBLE3x4 = "double3x4";
  GXLPCSTR STR_DOUBLE4x2 = "double4x2";
  GXLPCSTR STR_DOUBLE4x3 = "double4x3";
  GXLPCSTR STR_DOUBLE4x4 = "double4x4";

  GXLPCSTR STR_VEC2   = "vec2";
  GXLPCSTR STR_VEC3   = "vec3";
  GXLPCSTR STR_VEC4   = "vec4";
  GXLPCSTR STR_IVEC2  = "ivec2";
  GXLPCSTR STR_IVEC3  = "ivec3";
  GXLPCSTR STR_IVEC4  = "ivec4";
  GXLPCSTR STR_UVEC2  = "uvec2";
  GXLPCSTR STR_UVEC3  = "uvec3";
  GXLPCSTR STR_UVEC4  = "uvec4";
  GXLPCSTR STR_MAT2   = "mat2";  // 两行两列
  GXLPCSTR STR_MAT2x2 = "mat2x2";
  GXLPCSTR STR_MAT3   = "mat3"; 	// 三行三列
  GXLPCSTR STR_MAT3x3 = "mat3x3";
  GXLPCSTR STR_MAT4   = "mat4"; 	// 四行四列
  GXLPCSTR STR_MAT4x4 = "mat4x4";
  GXLPCSTR STR_MAT2x3 = "mat2x3";	// 三行两列
  GXLPCSTR STR_MAT2x4 = "mat2x4";	// 四行两列
  GXLPCSTR STR_MAT3x2 = "mat3x2";	// 两行三列
  GXLPCSTR STR_MAT3x4 = "mat3x4";	// 四行三列
  GXLPCSTR STR_MAT4x2 = "mat4x2";	// 两行四列
  GXLPCSTR STR_MAT4x3 = "mat4x3";	// 三行四列

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
    {STR_INT, "x"},
    {STR_INT, "r"},
    {STR_INT, "y"},
    {STR_INT, "g"},
    {STR_INT, "z"},
    {STR_INT, "b"},
    {STR_INT, "w"},
    {STR_INT, "a"},
  };

  MEMBERLIST s_aVecXYZW[] = {
    {STR_FLOAT, "x"},
    {STR_FLOAT, "r"},
    {STR_FLOAT, "y"},
    {STR_FLOAT, "g"},
    {STR_FLOAT, "z"},
    {STR_FLOAT, "b"},
    {STR_FLOAT, "w"},
    {STR_FLOAT, "a"},
  };

  MEMBERLIST s_aHalfXYZW[] = {
    {STR_HALF, "x"},
    {STR_HALF, "r"},
    {STR_HALF, "y"},
    {STR_HALF, "g"},
    {STR_HALF, "z"},
    {STR_HALF, "b"},
    {STR_HALF, "w"},
    {STR_HALF, "a"},
  };

  MEMBERLIST s_aUintXYZW[] = {
    {STR_UINT, "x"},
    {STR_UINT, "r"},
    {STR_UINT, "y"},
    {STR_UINT, "g"},
    {STR_UINT, "z"},
    {STR_UINT, "b"},
    {STR_UINT, "w"},
    {STR_UINT, "a"},
  };

  MEMBERLIST s_aDoubleXYZW[] = {
    {STR_DOUBLE, "x"},
    {STR_DOUBLE, "r"},
    {STR_DOUBLE, "y"},
    {STR_DOUBLE, "g"},
    {STR_DOUBLE, "z"},
    {STR_DOUBLE, "b"},
    {STR_DOUBLE, "w"},
    {STR_DOUBLE, "a"},
  };

  COMMINTRTYPEDESC s_aIntrinsicStruct[] =
  {
    {"int2", VALUE::Rank_Undefined, s_aIntXYZW, 4, STR_INT, "xy", "rg"},
    {"int3", VALUE::Rank_Undefined, s_aIntXYZW, 6, STR_INT, "xyz", "rgb"},
    {"int4", VALUE::Rank_Undefined, s_aIntXYZW, 8, STR_INT, "xyzw", "rgba"},
    
    {"vec2", VALUE::Rank_Undefined, s_aVecXYZW, 4, STR_FLOAT, "xy", "rg"},
    {"vec3", VALUE::Rank_Undefined, s_aVecXYZW, 6, STR_FLOAT, "xyz", "rgb"},
    {"vec4", VALUE::Rank_Undefined, s_aVecXYZW, 8, STR_FLOAT, "xyzw", "rgba"},
    
    {"half2", VALUE::Rank_Undefined, s_aHalfXYZW, 4, STR_HALF, "xy", "rg"},
    {"half3", VALUE::Rank_Undefined, s_aHalfXYZW, 6, STR_HALF, "xyz", "rgb"},
    {"half4", VALUE::Rank_Undefined, s_aHalfXYZW, 8, STR_HALF, "xyzw", "rgba"},

    {"uint2", VALUE::Rank_Undefined, s_aUintXYZW, 4, STR_UINT, "xy", "rg"},
    {"uint3", VALUE::Rank_Undefined, s_aUintXYZW, 6, STR_UINT, "xyz", "rgb"},
    {"uint4", VALUE::Rank_Undefined, s_aUintXYZW, 8, STR_UINT, "xyzw", "rgba"},

    {"float2", VALUE::Rank_Undefined, s_aVecXYZW, 4, STR_FLOAT, "xy", "rg"},
    {"float3", VALUE::Rank_Undefined, s_aVecXYZW, 6, STR_FLOAT, "xyz", "rgb"},
    {"float4", VALUE::Rank_Undefined, s_aVecXYZW, 8, STR_FLOAT, "xyzw", "rgba"},
    
    {"double2", VALUE::Rank_Undefined, s_aDoubleXYZW, 4, STR_DOUBLE, "xy", "rg"},
    {"double3", VALUE::Rank_Undefined, s_aDoubleXYZW, 6, STR_DOUBLE, "xyz", "rgb"},
    {"double4", VALUE::Rank_Undefined, s_aDoubleXYZW, 8, STR_DOUBLE, "xyzw", "rgba"},

    {NULL},
  };

  COMMINTRTYPEDESC s_aBaseType[] =
  {
    {STR_FLOAT, VALUE::Rank_Float},
    {STR_HALF, VALUE::Rank_Float},
    {STR_INT, VALUE::Rank_Signed},
    {STR_BOOL, VALUE::Rank_Undefined},
    {STR_UINT, VALUE::Rank_Unsigned},
    {STR_DOUBLE, VALUE::Rank_Double},
    {NULL}
  };

  //static GXLPCSTR s_ParamArray_Float2[] = { "float2", "float2", "float2", "float2" };
  //static GXLPCSTR s_ParamArray_Float3[] = { "float3", "float3", "float3", "float3" };
  //static GXLPCSTR s_ParamArray_Float4[] = { "float4", "float4", "float4", "float4" };
  //static GXLPCSTR s_ParamArray_Vec2[] = { "vec2", "vec2", "vec2", "vec2" };
  //static GXLPCSTR s_ParamArray_Vec3[] = { "vec3", "vec3", "vec3", "vec3" };
  //static GXLPCSTR s_ParamArray_Vec4[] = { "vec4", "vec4", "vec4", "vec4" };

  INTRINSIC_FUNC s_functions[] =
  {
    // 位含义: 00C32MVS
    // M: Matrix
    // V: Vector
    // S: Scaler
    // 2: Sampler2D
    // 3: Sampler3D
    // C: SamplerCube
    {"sqrt", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"sin", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"cos", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"asin", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"acos", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"floor", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"dot", INTRINSIC_FUNC::RetType_Scaler0, 2, "\x02\x02"},
    {"cross", INTRINSIC_FUNC::RetType_Argument0, 2, "\x02\x02"},
    {"normalize", INTRINSIC_FUNC::RetType_Argument0, 1, "\x02"},
    {"frac", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"clamp", INTRINSIC_FUNC::RetType_Argument0, 3, "\x07\x07\x07"},
    {"length", INTRINSIC_FUNC::RetType_Scaler0, 1, "\x02"},
    
    // HLSL
    {"float3", INTRINSIC_FUNC::RetType_Argument0, 1, "\x02"},
    {"lerp", INTRINSIC_FUNC::RetType_Argument0, 3, "\x07\x07\x07"}, // FIXME: 没有验证参数的一致性
    
    // GLSL
    {"vec2", INTRINSIC_FUNC::RetType_Argument0, 1, "\x02"},
    {"vec2", INTRINSIC_FUNC::RetType_FromName, 2, "\x01\x01"},
    {"vec3", INTRINSIC_FUNC::RetType_Argument0, 1, "\x02"},
    {"vec3", INTRINSIC_FUNC::RetType_FromName, 3, "\x01\x01\x01"},
    {"vec4", INTRINSIC_FUNC::RetType_Argument0, 1, "\x02"},
    {"vec4", INTRINSIC_FUNC::RetType_FromName, 4, "\x01\x01\x01\x01"},
    {"mix", INTRINSIC_FUNC::RetType_Argument0, 3, "\x03\x03\x01"}, // FIXME: 没有验证第一个参数和第二个参数类型相同
    {"fract", INTRINSIC_FUNC::RetType_Argument0, 1, "\x07"},
    {"texture2D", INTRINSIC_FUNC::RetType_Vector4, 2, "\x08\x02"}, // FIXME: 2维向量
    {"texture2D", INTRINSIC_FUNC::RetType_Vector4, 3, "\x08\x02\x01"}, // FIXME: 2维向量
    {NULL},
  };

  // 不对外使用
  static GXLPCSTR s_Vec4_ParamArray[] = { STR_VEC3, STR_FLOAT };
  static GXLPCSTR s_ParamArray_Floats[] = { STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT };
  INTRINSIC_FUNC2 s_functions2[] =
  {
    {"vec4", "vec4", 2, s_Vec4_ParamArray},
    {"float", "float", 1, s_ParamArray_Floats},
    {NULL},
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


