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
  GXLPCSTR STR_BOOL   = "bool";
  GXLPCSTR STR_FLOAT  = "float";
  GXLPCSTR STR_FLOAT2 = "float2";
  GXLPCSTR STR_FLOAT3 = "float3";
  GXLPCSTR STR_FLOAT4 = "float4";

  GXLPCSTR STR_INT2 = "int2";
  GXLPCSTR STR_INT3 = "int3";
  GXLPCSTR STR_INT4 = "int4";

  GXLPCSTR STR_UINT2 = "uint2";
  GXLPCSTR STR_UINT3 = "uint3";
  GXLPCSTR STR_UINT4 = "uint4";

  GXLPCSTR STR_HALF = "half";
  GXLPCSTR STR_HALF2 = "half2";
  GXLPCSTR STR_HALF3 = "half3";
  GXLPCSTR STR_HALF4 = "half4";

  GXLPCSTR STR_BOOL2 = "bool2";
  GXLPCSTR STR_BOOL3 = "bool3";
  GXLPCSTR STR_BOOL4 = "bool4";

  GXLPCSTR STR_INT1x1 = "int1x1";
  GXLPCSTR STR_INT1x2 = "int1x2";
  GXLPCSTR STR_INT1x3 = "int1x3";
  GXLPCSTR STR_INT1x4 = "int1x4";
  GXLPCSTR STR_INT2x1 = "int2x1";
  GXLPCSTR STR_INT2x2 = "int2x2";
  GXLPCSTR STR_INT2x3 = "int2x3";
  GXLPCSTR STR_INT2x4 = "int2x4";
  GXLPCSTR STR_INT3x1 = "int3x1";
  GXLPCSTR STR_INT3x2 = "int3x2";
  GXLPCSTR STR_INT3x3 = "int3x3";
  GXLPCSTR STR_INT3x4 = "int3x4";
  GXLPCSTR STR_INT4x1 = "int4x1";
  GXLPCSTR STR_INT4x2 = "int4x2";
  GXLPCSTR STR_INT4x3 = "int4x3";
  GXLPCSTR STR_INT4x4 = "int4x4";

  GXLPCSTR STR_UINT1x1 = "uint1x1";
  GXLPCSTR STR_UINT1x2 = "uint1x2";
  GXLPCSTR STR_UINT1x3 = "uint1x3";
  GXLPCSTR STR_UINT1x4 = "uint1x4";
  GXLPCSTR STR_UINT2x1 = "uint2x1";
  GXLPCSTR STR_UINT2x2 = "uint2x2";
  GXLPCSTR STR_UINT2x3 = "uint2x3";
  GXLPCSTR STR_UINT2x4 = "uint2x4";
  GXLPCSTR STR_UINT3x1 = "uint3x1";
  GXLPCSTR STR_UINT3x2 = "uint3x2";
  GXLPCSTR STR_UINT3x3 = "uint3x3";
  GXLPCSTR STR_UINT3x4 = "uint3x4";
  GXLPCSTR STR_UINT4x1 = "uint4x1";
  GXLPCSTR STR_UINT4x2 = "uint4x2";
  GXLPCSTR STR_UINT4x3 = "uint4x3";
  GXLPCSTR STR_UINT4x4 = "uint4x4";

  GXLPCSTR STR_HALF1x1 = "half1x1";
  GXLPCSTR STR_HALF1x2 = "half1x2";
  GXLPCSTR STR_HALF1x3 = "half1x3";
  GXLPCSTR STR_HALF1x4 = "half1x4";
  GXLPCSTR STR_HALF2x1 = "half2x1";
  GXLPCSTR STR_HALF2x2 = "half2x2";
  GXLPCSTR STR_HALF2x3 = "half2x3";
  GXLPCSTR STR_HALF2x4 = "half2x4";
  GXLPCSTR STR_HALF3x1 = "half3x1";
  GXLPCSTR STR_HALF3x2 = "half3x2";
  GXLPCSTR STR_HALF3x3 = "half3x3";
  GXLPCSTR STR_HALF3x4 = "half3x4";
  GXLPCSTR STR_HALF4x1 = "half4x1";
  GXLPCSTR STR_HALF4x2 = "half4x2";
  GXLPCSTR STR_HALF4x3 = "half4x3";
  GXLPCSTR STR_HALF4x4 = "half4x4";

  GXLPCSTR STR_FLOAT1x1 = "float1x1";
  GXLPCSTR STR_FLOAT1x2 = "float1x2";
  GXLPCSTR STR_FLOAT1x3 = "float1x3";
  GXLPCSTR STR_FLOAT1x4 = "float1x4";
  GXLPCSTR STR_FLOAT2x1 = "float2x1";
  GXLPCSTR STR_FLOAT2x2 = "float2x2";
  GXLPCSTR STR_FLOAT2x3 = "float2x3";
  GXLPCSTR STR_FLOAT2x4 = "float2x4";  
  GXLPCSTR STR_FLOAT3x1 = "float3x1";
  GXLPCSTR STR_FLOAT3x2 = "float3x2";
  GXLPCSTR STR_FLOAT3x3 = "float3x3";
  GXLPCSTR STR_FLOAT3x4 = "float3x4";
  GXLPCSTR STR_FLOAT4x1 = "float4x1";
  GXLPCSTR STR_FLOAT4x2 = "float4x2";
  GXLPCSTR STR_FLOAT4x3 = "float4x3";
  GXLPCSTR STR_FLOAT4x4 = "float4x4";

  GXLPCSTR STR_DOUBLE  = "double";
  GXLPCSTR STR_DOUBLE2 = "double2";
  GXLPCSTR STR_DOUBLE3 = "double3";
  GXLPCSTR STR_DOUBLE4 = "double4";

  GXLPCSTR STR_DOUBLE1x1 = "double1x1";
  GXLPCSTR STR_DOUBLE1x2 = "double1x2";
  GXLPCSTR STR_DOUBLE1x3 = "double1x3";
  GXLPCSTR STR_DOUBLE1x4 = "double1x4";
  GXLPCSTR STR_DOUBLE2x1 = "double2x1";
  GXLPCSTR STR_DOUBLE2x2 = "double2x2";
  GXLPCSTR STR_DOUBLE2x3 = "double2x3";
  GXLPCSTR STR_DOUBLE2x4 = "double2x4";
  GXLPCSTR STR_DOUBLE3x1 = "double3x1";
  GXLPCSTR STR_DOUBLE3x2 = "double3x2";
  GXLPCSTR STR_DOUBLE3x3 = "double3x3";
  GXLPCSTR STR_DOUBLE3x4 = "double3x4";
  GXLPCSTR STR_DOUBLE4x1 = "double4x1";
  GXLPCSTR STR_DOUBLE4x2 = "double4x2";
  GXLPCSTR STR_DOUBLE4x3 = "double4x3";
  GXLPCSTR STR_DOUBLE4x4 = "double4x4";

  //GXLPCSTR STR_VEC2   = "vec2";
  //GXLPCSTR STR_VEC3   = "vec3";
  //GXLPCSTR STR_VEC4   = "vec4";
  //GXLPCSTR STR_IVEC2  = "ivec2";
  //GXLPCSTR STR_IVEC3  = "ivec3";
  //GXLPCSTR STR_IVEC4  = "ivec4";
  //GXLPCSTR STR_UVEC2  = "uvec2";
  //GXLPCSTR STR_UVEC3  = "uvec3";
  //GXLPCSTR STR_UVEC4  = "uvec4";
  //GXLPCSTR STR_MAT2   = "mat2";  // 两行两列
  //GXLPCSTR STR_MAT2x2 = "mat2x2";
  //GXLPCSTR STR_MAT3   = "mat3";   // 三行三列
  //GXLPCSTR STR_MAT3x3 = "mat3x3";
  //GXLPCSTR STR_MAT4   = "mat4";   // 四行四列
  //GXLPCSTR STR_MAT4x4 = "mat4x4";
  //GXLPCSTR STR_MAT2x3 = "mat2x3";  // 三行两列
  //GXLPCSTR STR_MAT2x4 = "mat2x4";  // 四行两列
  //GXLPCSTR STR_MAT3x2 = "mat3x2";  // 两行三列
  //GXLPCSTR STR_MAT3x4 = "mat3x4";  // 四行三列
  //GXLPCSTR STR_MAT4x2 = "mat4x2";  // 两行四列
  //GXLPCSTR STR_MAT4x3 = "mat4x3";  // 三行四列

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
    {1, "^", OPP( 5)}, // 不是一元操作符啊！！ TRUE, UNARY_RIGHT_OPERAND},
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

  int GetVectorComponentIndex(TOKEN::TChar c)
  {
    switch(c)
    {
    case 'x':
    case 'r':
    case 's':
      return 0;
    case 'y':
    case 'g':
    case 't':
      return 1;
    case 'z':
    case 'b':
    case 'p':
      return 2;
    case 'w':
    case 'a':
    case 'q':
      return 3;

    default:
      CLBREAK; // 外部保证
      break;
    }
    return -1;
  }

#define GET_VECTOR_COMPONENT_INDEX(_N)  pResult->components[_N] = GetVectorComponentIndex(p[_N]);
  GXBOOL EndVectorComponentOperator(const COMMINTRTYPEDESC* pDesc, DOTOPERATOR_RESULT* pResult, const TOKEN* pToken)
  {
    if(pToken->length == 1) {
      pResult->strType = pDesc->component_type;
      pResult->components[pToken->length] = -1;
      return TRUE;
    }
    else if(pToken->length > 4) {
      return FALSE;
    }

    pResult->strType = pDesc->component_type;
    pResult->strType.Append('0' + pToken->length);
    pResult->components[pToken->length] = -1;
    return TRUE;
  }

  GXBOOL GXCALLBACK OnVector2(const COMMINTRTYPEDESC* pDesc, DOTOPERATOR_RESULT* pResult, const TOKEN* pToken)
  {
    TOKEN::T_LPCSTR p = pToken->marker;
    if(p[0] == 'x' || p[0] == 'y') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 'x' || p[i] == 'y')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else if(p[0] == 'r' || p[0] == 'g') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 'r' || p[i] == 'g')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else if(p[0] == 's' || p[0] == 't') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 's' || p[i] == 't')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else {
      return FALSE;
    }
    return EndVectorComponentOperator(pDesc, pResult, pToken);
  }

  GXBOOL GXCALLBACK OnVector3(const COMMINTRTYPEDESC* pDesc, DOTOPERATOR_RESULT* pResult, const TOKEN* pToken)
  {
    TOKEN::T_LPCSTR p = pToken->marker;
    if(p[0] == 'x' || p[0] == 'y' || p[0] == 'z') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 'x' || p[i] == 'y' || p[i] == 'z')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else if(p[0] == 'r' || p[0] == 'g' || p[0] == 'b') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 'r' || p[i] == 'g' || p[i] == 'b')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else if(p[0] == 's' || p[0] == 't' || p[0] == 'p') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 's' || p[i] == 't' || p[i] == 'p')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else {
      return FALSE;
    }

    return EndVectorComponentOperator(pDesc, pResult, pToken);
  }

  GXBOOL GXCALLBACK OnVector4(const COMMINTRTYPEDESC* pDesc, DOTOPERATOR_RESULT* pResult, const TOKEN* pToken)
  {
    TOKEN::T_LPCSTR p = pToken->marker;
    if(p[0] == 'x' || p[0] == 'y' || p[0] == 'z' || p[0] == 'w') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 'x' || p[i] == 'y' || p[i] == 'z' || p[i] == 'w')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else if(p[0] == 'r' || p[0] == 'g' || p[0] == 'b' || p[0] == 'a') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 'r' || p[i] == 'g' || p[i] == 'b' || p[i] == 'a')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else if(p[0] == 's' || p[0] == 't' || p[0] == 'p' || p[0] == 'q') {
      GET_VECTOR_COMPONENT_INDEX(0);
      for(size_t i = 1; i < pToken->length; i++) {
        if(_CL_NOT_(p[i] == 's' || p[i] == 't' || p[i] == 'p' || p[i] == 'q')) {
          return FALSE;
        }
        GET_VECTOR_COMPONENT_INDEX(i);
      }
    }
    else {
      return FALSE;
    }

    return EndVectorComponentOperator(pDesc, pResult, pToken);
  }

  //const TYPEDESC* GXCALLBACK OnFloatVectorSubscript(VALUE_CONTEXT& vctx, const VALUE* pValue)
  //{
  //  clStringA str = vctx.pType->name;
  //  if(str == STR_FLOAT2 || str == STR_FLOAT3 || str == STR_FLOAT4) {
  //    return sNameCtx.GetType(STR_FLOAT);
  //  }
  //  else if(str == STR_HALF2 || str == STR_HALF3 || str == STR_HALF4) {
  //    return sNameCtx.GetType(STR_HALF);
  //  }
  //  else if(str == STR_DOUBLE2 || str == STR_DOUBLE3 || str == STR_DOUBLE4) {
  //    return sNameCtx.GetType(STR_DOUBLE);
  //  }
  //  CLBREAK;
  //  return NULL;
  //}

  //const TYPEDESC* GXCALLBACK OnMatrixSubscript(VALUE_CONTEXT& vctx, const VALUE* pValue)
  //{
  //  clStringA str = pDesc->name;
  //  ASSERT(str.EndsWith("x1") || str.EndsWith("x2") || str.EndsWith("x3") || str.EndsWith("x4"));
  //  str.Remove(str.GetLength() - 2, 2);
  //  str.TrimRight('1');
  //  return sNameCtx.GetType(str);
  //}

  const TYPEDESC* GXCALLBACK OnVectorSubscript(VALUE_CONTEXT& vctx, const VALUE* pValue)
  {
    CHECK_VALUE_CONTEXT;
    ASSERT(pValue == NULL || (pValue->rank == VALUE::Rank_Signed || pValue->rank == VALUE::Rank_Signed64 ||
      pValue->rank == VALUE::Rank_Unsigned || pValue->rank == VALUE::Rank_Unsigned64));
    ASSERT(vctx.pType->cate == TYPEDESC::TypeCate_Vector ||
      vctx.pType->cate == TYPEDESC::TypeCate_Matrix);
    ASSERT(pValue == NULL || pValue->IsNegative() == FALSE);
    const size_t element_size = vctx.pType->pElementType->CountOf();
    const size_t element_count = vctx.pType->CountOf() / element_size;
    if(pValue && (pValue->IsNegative() || pValue->uValue64 >= element_count)) {
      CHECK_VALUE_CONTEXT_CLEARERRORCOUNT;
      return vctx.ClearValue(ValueResult_SubscriptOutOfRange).pType;
    }

    vctx.pType = vctx.pType->pElementType;
    if(vctx.pValue && pValue)
    {
      if(vctx.pool.empty())
      {
        vctx.pValue += element_size * pValue->uValue64;
        vctx.count = element_size;
      }
      else
      {
        ValuePool temp(vctx.pValue + element_size * pValue->uValue64, vctx.pValue + element_size * (pValue->uValue64 + 1));
        vctx.pool.assign(temp.begin(), temp.end());
        vctx.UsePool();
      }
    }
    else if(pValue == NULL) { // 写的稍微严谨一点排除了vctx.pValue为NULL的情况
      vctx.ClearValueOnly();
    }
    return vctx.pType;
  }

  const TYPEDESC* GXCALLBACK OnMatrixSubscript(VALUE_CONTEXT& vctx, const VALUE* pValue)
  {
    CHECK_VALUE_CONTEXT;
    ASSERT(pValue == NULL || (pValue->rank == VALUE::Rank_Signed || pValue->rank == VALUE::Rank_Signed64 ||
      pValue->rank == VALUE::Rank_Unsigned || pValue->rank == VALUE::Rank_Unsigned64));
    ASSERT(vctx.pType->cate == TYPEDESC::TypeCate_Vector ||
      vctx.pType->cate == TYPEDESC::TypeCate_Matrix);
    ASSERT(pValue == NULL || pValue->IsNegative() == FALSE);

    clStringA str = vctx.pType->name;
    ASSERT(str.EndsWith("x1") || str.EndsWith("x2") || str.EndsWith("x3") || str.EndsWith("x4"));
    str.Remove(str.GetLength() - 2, 2);
    str.TrimRight('1');
    const TYPEDESC* pElementType = vctx.name_ctx.GetType(str);

    const size_t column_size = pElementType->CountOf();
    const size_t row_size = vctx.pType->CountOf() / column_size;
    if(pValue && (pValue->IsNegative() || pValue->uValue64 >= row_size)) {
      CHECK_VALUE_CONTEXT_CLEARERRORCOUNT;
      return vctx.ClearValue(ValueResult_SubscriptOutOfRange).pType;
    }

    vctx.pType = pElementType;
    if(vctx.pValue && pValue)
    {
      if(vctx.pool.empty())
      {
        vctx.pValue += column_size * pValue->uValue64;
        vctx.count = column_size;
      }
      else
      {
        ValuePool temp(vctx.pValue + column_size * pValue->uValue64, vctx.pValue + column_size * (pValue->uValue64 + 1));
        vctx.pool.assign(temp.begin(), temp.end());
        vctx.UsePool();
      }
    }
    else if(pValue == NULL) { // 写的稍微严谨一点排除了vctx.pValue为NULL的情况
      vctx.ClearValueOnly();
    }
    return vctx.pType;
  }

  COMMINTRTYPEDESC s_aIntrinsicStruct[] =
  {
    {STR_INT2, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Vector, OnVector2, OnVectorSubscript},
    {STR_INT3, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Vector, OnVector3, OnVectorSubscript},
    {STR_INT4, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Vector, OnVector4, OnVectorSubscript},

    {STR_BOOL2, STR_BOOL, VALUE::Rank_Signed, TYPEDESC::TypeCate_Vector, OnVector2, OnVectorSubscript},
    {STR_BOOL3, STR_BOOL, VALUE::Rank_Signed, TYPEDESC::TypeCate_Vector, OnVector3, OnVectorSubscript},
    {STR_BOOL4, STR_BOOL, VALUE::Rank_Signed, TYPEDESC::TypeCate_Vector, OnVector4, OnVectorSubscript},

    {STR_HALF2, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Vector, OnVector2, OnVectorSubscript},
    {STR_HALF3, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Vector, OnVector3, OnVectorSubscript},
    {STR_HALF4, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Vector, OnVector4, OnVectorSubscript},

    {STR_UINT2, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Vector, OnVector2, OnVectorSubscript},
    {STR_UINT3, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Vector, OnVector3, OnVectorSubscript},
    {STR_UINT4, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Vector, OnVector4, OnVectorSubscript},

    {STR_FLOAT2, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Vector, OnVector2, OnVectorSubscript},
    {STR_FLOAT3, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Vector, OnVector3, OnVectorSubscript},
    {STR_FLOAT4, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Vector, OnVector4, OnVectorSubscript},

    {STR_DOUBLE2, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Vector, OnVector2, OnVectorSubscript},
    {STR_DOUBLE3, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Vector, OnVector3, OnVectorSubscript},
    {STR_DOUBLE4, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Vector, OnVector4, OnVectorSubscript},

    {STR_INT1x1, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT1x2, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT1x3, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT1x4, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT2x1, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT2x2, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT2x3, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT2x4, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT3x1, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT3x2, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT3x3, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT3x4, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT4x1, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT4x2, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT4x3, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_INT4x4, STR_INT, VALUE::Rank_Signed, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},

    {STR_UINT1x1, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT1x2, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT1x3, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT1x4, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT2x1, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT2x2, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT2x3, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT2x4, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT3x1, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT3x2, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT3x3, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT3x4, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT4x1, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT4x2, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT4x3, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_UINT4x4, STR_UINT, VALUE::Rank_Unsigned, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},

    {STR_HALF1x1, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF1x2, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF1x3, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF1x4, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF2x1, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF2x2, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF2x3, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF2x4, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF3x1, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF3x2, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF3x3, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF3x4, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF4x1, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF4x2, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF4x3, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_HALF4x4, STR_HALF, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},

    {STR_FLOAT1x1, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT1x2, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT1x3, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT1x4, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT2x1, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT2x2, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT2x3, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT2x4, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT3x1, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT3x2, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT3x3, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT3x4, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT4x1, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT4x2, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT4x3, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_FLOAT4x4, STR_FLOAT, VALUE::Rank_Float, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},

    {STR_DOUBLE1x1, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE1x2, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE1x3, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE1x4, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE2x1, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE2x2, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE2x3, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE2x4, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE3x1, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE3x2, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE3x3, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE3x4, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE4x1, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE4x2, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE4x3, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},
    {STR_DOUBLE4x4, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_Matrix, NULL, OnMatrixSubscript},

    {NULL},
  };

  COMMINTRTYPEDESC s_aBaseType[] =
  {
    {STR_HALF,   STR_HALF,   VALUE::Rank_Float, TYPEDESC::TypeCate_FloatScaler},
    {STR_FLOAT,  STR_FLOAT,  VALUE::Rank_Float, TYPEDESC::TypeCate_FloatScaler},
    {STR_DOUBLE, STR_DOUBLE, VALUE::Rank_Double, TYPEDESC::TypeCate_FloatScaler},
    {STR_INT,    STR_INT,    VALUE::Rank_Signed, TYPEDESC::TypeCate_IntegerScaler},
    {STR_BOOL,   STR_BOOL,   VALUE::Rank_Signed, TYPEDESC::TypeCate_IntegerScaler},
    {STR_UINT,   STR_UINT,   VALUE::Rank_Unsigned, TYPEDESC::TypeCate_IntegerScaler},
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
    // 高位含义: 
    // +-----------------------+
    // |15|14|13|12|11|10| 9| 8|
    // +-----------------------+
    // | tempid | I| B| ------ |
    // +-----------------------+
    // tempid: 模板类型id，相同id的形参类型也相同
    // I:Scaler/Vector/Matrix的分量可以是整数类型（浮点类型默认支持）
    // B:Scaler/Vector/Matrix的分量可以是布尔类型

    // 低位含义: OC321MVS
    // M: Matrix
    // V: Vector
    // S: Scaler
    // 1: Sampler1D
    // 2: Sampler2D
    // 3: Sampler3D
    // C: SamplerCube
    // O: out
#define ARG_MatVecSca    "\x07\x00"
#define ARG_VecScal      "\x03\x00"
#define ARG_Vector       "\x02\x00"
#define ARG_Scaler       "\x01\x00"
#define ARG_Sampler1D    "\x08\x00"
#define ARG_Sampler2D    "\x10\x00"
#define ARG_Sampler3D    "\x20\x00"
#define ARG_SamplerCube  "\x40\x00"
#define ARG_OutMatVecSca "\x87\x00"

    {"abs", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"acos", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"all", INTRINSIC_FUNC::RetType_Bool, 1, (u16*)ARG_MatVecSca},
    {"any", INTRINSIC_FUNC::RetType_Bool, 1, (u16*)ARG_MatVecSca},
    {"sqrt", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"sin", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"cos", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"asin", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"floor", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"frac", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"exp", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"length", INTRINSIC_FUNC::RetType_Scaler0, 1, (u16*)ARG_Vector},
    {"noise", INTRINSIC_FUNC::RetType_Scaler0, 1, (u16*)ARG_Vector},
    {"normalize", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_Vector},
    {"atan", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"tan", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"log", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"log2", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"exp2", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"sign", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"degrees", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"ceil", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"round", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"trunc", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"isinf", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"isnan", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"radians", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"fwidth", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"atan", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"tanh", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"cosh", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"sinh", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},

    {"reflect", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_Vector ARG_Vector},
    {"atan2", INTRINSIC_FUNC::RetType_Argument1, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},
    {"dot", INTRINSIC_FUNC::RetType_Scaler0, 2, (u16*)ARG_Vector ARG_Vector},
    {"distance", INTRINSIC_FUNC::RetType_Scaler0, 2, (u16*)ARG_VecScal ARG_VecScal},
    {"cross", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_Vector ARG_Vector},
    {"min", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},
    {"max", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},
    {"step", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},
    {"modf", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},
    {"pow", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},

    {"clamp", INTRINSIC_FUNC::RetType_Argument0, 3, (u16*)ARG_MatVecSca ARG_MatVecSca ARG_MatVecSca},
    {"smoothstep", INTRINSIC_FUNC::RetType_Argument0, 3, (u16*)ARG_MatVecSca ARG_MatVecSca ARG_MatVecSca}, // FIXME: 没有验证参数的一致性
    {"sincos", INTRINSIC_FUNC::RetType_Argument0, 3, (u16*)ARG_MatVecSca ARG_OutMatVecSca ARG_OutMatVecSca},
    {"refract", INTRINSIC_FUNC::RetType_Argument0, 3, (u16*)ARG_Vector ARG_Vector ARG_Scaler},

    // HLSL
    {"lerp", INTRINSIC_FUNC::RetType_Argument0, 3, (u16*)ARG_MatVecSca ARG_MatVecSca ARG_MatVecSca}, // FIXME: 没有验证参数的一致性

    // GLSL 将来去掉
    {"fract", INTRINSIC_FUNC::RetType_Argument0, 1, (u16*)ARG_MatVecSca},
    {"mix", INTRINSIC_FUNC::RetType_Argument0, 3, (u16*)ARG_VecScal ARG_VecScal ARG_Scaler}, // FIXME: 没有验证第一个参数和第二个参数类型相同
    {"tex2D", INTRINSIC_FUNC::RetType_Float4, 2, (u16*)ARG_Sampler2D ARG_Vector}, // FIXME: 2维向量
    {"tex3D", INTRINSIC_FUNC::RetType_Float4, 2, (u16*)ARG_Sampler3D ARG_Vector}, // FIXME: 3维向量
    {"tex2D", INTRINSIC_FUNC::RetType_Float4, 3, (u16*)ARG_Sampler2D ARG_Vector ARG_Scaler}, // FIXME: 2维向量
    {"tex2DBias", INTRINSIC_FUNC::RetType_Float4, 2, (u16*)ARG_Sampler2D ARG_Vector}, // FIXME: 4维向量
    {"texCUBE", INTRINSIC_FUNC::RetType_Float4, 2, (u16*)ARG_SamplerCube ARG_Vector},
    {"mod", INTRINSIC_FUNC::RetType_Argument0, 2, (u16*)ARG_MatVecSca ARG_MatVecSca},
    {NULL},
  };

  // 不对外使用
  //static GXLPCSTR s_Vec3_ParamArray0[] = { STR_FLOAT2, STR_FLOAT };
  //static GXLPCSTR s_Vec3_ParamArray1[] = { STR_FLOAT, STR_FLOAT2 };
  //static GXLPCSTR s_ParamArray_v2v2[] = { STR_FLOAT2, STR_FLOAT2};
  //static GXLPCSTR s_ParamArray_v3v3v3[] = { STR_FLOAT3, STR_FLOAT3, STR_FLOAT3 };
  //static GXLPCSTR s_ParamArray_v4v4v4v4[] = { STR_FLOAT4, STR_FLOAT4, STR_FLOAT4, STR_FLOAT4 };
  //static GXLPCSTR s_Vec4_ParamArray_v3s[] = { STR_FLOAT3, STR_FLOAT };
  //static GXLPCSTR s_Vec4_ParamArray_v2ss[] = { STR_FLOAT2, STR_FLOAT, STR_FLOAT };
  //static GXLPCSTR s_Vec4_ParamArray_sv2s[] = { STR_FLOAT, STR_FLOAT2, STR_FLOAT };
  //static GXLPCSTR s_Vec4_ParamArray_ssv2[] = { STR_FLOAT, STR_FLOAT, STR_FLOAT2 };
  //static GXLPCSTR s_ParamArray_Floats_16[] = { STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT, STR_FLOAT };

  //INTRINSIC_FUNC2 s_functions2[] =
  //{
  //  //{"vec3", "vec3", 2, s_Vec3_ParamArray0},
  //  //{"vec3", "vec3", 2, s_Vec3_ParamArray1},
  //  //{"vec4", "vec4", 2, s_Vec4_ParamArray},
  //  {"float2", "float2", 1, s_ParamArray_Floats_16},
  //  {"float3", "float3", 1, s_ParamArray_Floats_16},
  //  {"float4", "float4", 1, s_ParamArray_Floats_16},
  //  {"float2x2", "float2x2", 1, s_ParamArray_Floats_16},
  //  {"float3x3", "float3x3", 1, s_ParamArray_Floats_16},
  //  {"float4x4", "float4x4", 1, s_ParamArray_Floats_16},

  //  {"float2", "float2", 2, s_ParamArray_Floats_16},
  //  {"float3", "float3", 3, s_ParamArray_Floats_16},
  //  {"float4", "float4", 4, s_ParamArray_Floats_16},
  //  {"float2x2", "float2x2", 4, s_ParamArray_Floats_16},
  //  {"float3x3", "float3x3", 9, s_ParamArray_Floats_16},
  //  {"float4x4", "float4x4", 16, s_ParamArray_Floats_16},

  //  {"float3", "float3", 2, s_Vec3_ParamArray0},
  //  {"float3", "float3", 2, s_Vec3_ParamArray1},
  //  
  //  {"float4", "float4", 2, s_Vec4_ParamArray_v3s},
  //  {"float4", "float4", 3, s_Vec4_ParamArray_ssv2},
  //  {"float4", "float4", 3, s_Vec4_ParamArray_sv2s},
  //  {"float4", "float4", 3, s_Vec4_ParamArray_v2ss},
  //  {"float", "float", 1, s_ParamArray_Floats_16},
  //  {"vec2", "sincos", 1, s_ParamArray_Floats_16},

  //  {"float2x2", "float2x2", 2, s_ParamArray_v2v2},    
  //  {"float3x3", "float3x3", 3, s_ParamArray_v3v3v3},
  //  {"float4x4", "float4x4", 4, s_ParamArray_v4v4v4v4},
  //  {NULL},
  //};

  //// 类型扩充
  //PERCOMPONENTMATH s_TypeExtension[] = {
  //};

  // 用来描述向量和矩阵的初始化
  PERCOMPONENTMATH s_PreComponentMath[] = {
    {STR_FLOAT, 1},
    {STR_FLOAT2, 2},
    {STR_FLOAT3, 3},
    {STR_FLOAT4, 4},
    {STR_INT, 1},
    {STR_INT2, 2},
    {STR_INT3, 3},
    {STR_INT4, 4},
    {STR_UINT, 1},
    {STR_UINT2, 2},
    {STR_UINT3, 3},
    {STR_UINT4, 4},
    {STR_HALF, 1},
    {STR_HALF2, 2},
    {STR_HALF3, 3},
    {STR_HALF4, 4},
    {STR_DOUBLE, 1},
    {STR_BOOL, 1},
    {STR_BOOL2, 2},
    {STR_BOOL3, 3},
    {STR_BOOL4, 4},

    {STR_DOUBLE2, 2},
    {STR_DOUBLE3, 3},
    {STR_DOUBLE4, 4},

    {STR_INT2x2, 2 * 2},
    {STR_INT2x3, 2 * 3},
    {STR_INT2x4, 2 * 4},
    {STR_INT3x2, 3 * 2},
    {STR_INT3x3, 3 * 3},
    {STR_INT3x4, 3 * 4},
    {STR_INT4x2, 4 * 2},
    {STR_INT4x3, 4 * 3},
    {STR_INT4x4, 4 * 4},

    {STR_UINT2x2, 2 * 2},
    {STR_UINT2x3, 2 * 3},
    {STR_UINT2x4, 2 * 4},
    {STR_UINT3x2, 3 * 2},
    {STR_UINT3x3, 3 * 3},
    {STR_UINT3x4, 3 * 4},
    {STR_UINT4x2, 4 * 2},
    {STR_UINT4x3, 4 * 3},
    {STR_UINT4x4, 4 * 4},


    {STR_HALF2x2, 2 * 2},
    {STR_HALF2x3, 2 * 3},
    {STR_HALF2x4, 2 * 4},
    {STR_HALF3x2, 3 * 2},
    {STR_HALF3x3, 3 * 3},
    {STR_HALF3x4, 3 * 4},
    {STR_HALF4x2, 4 * 2},
    {STR_HALF4x3, 4 * 3},
    {STR_HALF4x4, 4 * 4},

    {STR_FLOAT2x2, 2 * 2},
    {STR_FLOAT2x3, 2 * 3},
    {STR_FLOAT2x4, 2 * 4},
    {STR_FLOAT3x2, 3 * 2},
    {STR_FLOAT3x3, 3 * 3},
    {STR_FLOAT3x4, 3 * 4},
    {STR_FLOAT4x2, 4 * 2},
    {STR_FLOAT4x3, 4 * 3},
    {STR_FLOAT4x4, 4 * 4},

    {STR_DOUBLE2x2, 2 * 2},
    {STR_DOUBLE2x3, 2 * 3},
    {STR_DOUBLE2x4, 2 * 4},
    {STR_DOUBLE3x2, 3 * 2},
    {STR_DOUBLE3x3, 3 * 3},
    {STR_DOUBLE3x4, 3 * 4},
    {STR_DOUBLE4x2, 4 * 2},
    {STR_DOUBLE4x3, 4 * 3},
    {STR_DOUBLE4x4, 4 * 4},

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


