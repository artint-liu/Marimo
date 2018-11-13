#include <tchar.h>
#include <Marimo.H>
#include "GrapX/DataPool.H"
#include <clTextLines.h>
#include "clPathFile.h"
//#include <vld.h>
#include "TestDataPool.h"

// Shader constant buffer打包规则参考：
// https://docs.microsoft.com/zh-cn/windows/desktop/direct3dhlsl/dx-graphics-hlsl-packing-rules

#define __STR__(_V) #_V
#define __STR2__(_V) __STR__(_V)
#define __LINE_STR__ __STR2__(__LINE__)
#define CHECK_VALUE(_VAR, _EXPR, _VAL) ASSERT((_VAR = _EXPR) == _VAL)

static GXLPCSTR g_szCodeSample = "#file" __FILE__ "\n\
#line " __LINE_STR__ "                          \n\
//  2 x 16byte elements                         \n\
struct IE1                                      \n\
{                                               \n\
  float4 Val1;                                  \n\
  float2 Val2;  // starts a new vector          \n\
  float2 Val3;                                  \n\
};                                              \n\
                                                \n\
//  3 x 16byte elements                         \n\
struct IE2                                      \n\
{                                               \n\
  float2 Val1;                                  \n\
  float4 Val2;  // starts a new vector          \n\
  float2 Val3;  // starts a new vector          \n\
};                                              \n\
                                                \n\
//  1 x 16byte elements                         \n\
struct IE3                                      \n\
{                                               \n\
  float Val1;                                   \n\
  float Val2;                                   \n\
  float2 Val3;                                  \n\
};                                              \n\
                                                \n\
//  1 x 16byte elements                         \n\
struct IE4                                      \n\
{                                               \n\
  float Val1;                                   \n\
  float2 Val2;                                  \n\
  float Val3;                                   \n\
};                                              \n\
                                                \n\
//  2 x 16byte elements                         \n\
struct IE5                                      \n\
{                                               \n\
  float Val1;                                   \n\
  float Val1_1;                                 \n\
  float Val1_2;                                 \n\
  float2 Val2;    // starts a new vector        \n\
};                                              \n\
                                                \n\
                                                \n\
//  1 x 16byte elements                         \n\
struct IE6                                      \n\
{                                               \n\
  float3 Val1;                                  \n\
  float Val2;                                   \n\
};                                              \n\
                                                \n\
//  1 x 16byte elements                         \n\
struct IE7                                      \n\
{                                               \n\
  float Val1;                                   \n\
  float3 Val2;                                  \n\
};                                              \n\
                                                \n\
//  2 x 16byte elements                         \n\
struct IE8                                      \n\
{                                               \n\
  float Val1;                                   \n\
  float Val1_1;                                 \n\
  float3 Val2;        // starts a new vector    \n\
};                                              \n\
                                                \n\
                                                \n\
// 3 x 16byte elements                          \n\
struct IE9_VAL {                                \n\
  float4 SVal1;    // starts a new vector       \n\
  float SVal2;    // starts a new vector        \n\
};                                              \n\
struct IE9                                      \n\
{                                               \n\
  float Val1;                                   \n\
                                                \n\
  IE9_VAL Val2;                                 \n\
};                                              \n\
                                                \n\
// 3 x 16byte elements                          \n\
struct IE10_VAL{                                \n\
  float SVal1;     // starts a new vector       \n\
  float4 SVal2;     // starts a new vector      \n\
};                                              \n\
struct IE10                                     \n\
{                                               \n\
  float Val1;                                   \n\
  IE10_VAL Val2;                                \n\
};                                              \n\
                                                \n\
// 3 x 16byte elements                          \n\
struct IE11_VAL {                               \n\
  float4 SVal1;                                 \n\
  float SVal2;    // starts a new vector        \n\
};                                              \n\
struct IE11                                     \n\
{                                               \n\
  IE11_VAL Val1;                                \n\
                                                \n\
  float Val2;   // starts a new vector          \n\
};                                              \n\
// ============================================ \n\
// 后面结构体是自己写的                          \n\
                                                \n\
struct IE12_VAL {                               \n\
  float3 SVal1;                                 \n\
  float2 SVal2;    // starts a new vector       \n\
  float2 SVal3;                                 \n\
  float  SVal4;    // starts a new vector       \n\
};                                              \n\
struct IE12                                     \n\
{                                               \n\
  IE12_VAL Val1;                                \n\
                                                \n\
  float Val2;   // starts a new vector          \n\
  float4 Val3;  // starts a new vector          \n\
  float2 Val4;  // starts a new vector          \n\
  float3 Val5;  // starts a new vector          \n\
};                                              \n\
                                                \n\
IE1 ie1;                                        \n\
IE2 ie2;                                        \n\
IE3 ie3;                                        \n\
IE4 ie4;                                        \n\
IE5 ie5;                                        \n\
IE6 ie6;                                        \n\
IE7 ie7;                                        \n\
IE8 ie8;                                        \n\
IE9 ie9;                                        \n\
IE10 ie10;                                      \n\
IE11 ie11;                                      \n\
IE12 ie12;                                      \n\
";

//////////////////////////////////////////////////////////////////////////
void TestSlidingArray(DataPool* pDataPool, DataPoolVariable& varArray, GXLPCSTR szVarFormat)
{
  TRACE("==== TestSlidingArray ====\n");
  ASSERT((varArray.GetCaps() & Marimo::DataPoolVariable::CAPS_ARRAY) ||  // 必须是数组
    (varArray.GetCaps() & Marimo::DataPoolVariable::CAPS_DYNARRAY));

  DataPoolVariable varRefElement, varSlidingElement;
  clStringA str;
  size_t val;
  const GXSIZE_T nCount = varArray.GetLength();

  for(GXSIZE_T i = 0; i < nCount; i++)
  {
    str.Format(szVarFormat, i);
    TRACE("Test sliding \"%s\"\n", str.CStr());
    pDataPool->QueryByExpression(str, &varRefElement);
    if(i == 0)
    {
      varSlidingElement = varRefElement;
    }

    CHECK_VALUE(val, varRefElement.GetBuffer() - varSlidingElement.GetBuffer(), 0);
    CHECK_VALUE(val, varRefElement.GetOffset() - varSlidingElement.GetOffset(), 0);
    CHECK_VALUE(val, (size_t)varRefElement.GetPtr() - (size_t)varSlidingElement.GetPtr(), 0);
    CHECK_VALUE(val, varRefElement.GetSize() - varSlidingElement.GetSize(), 0);
    varArray.Sliding(&varSlidingElement, 1);
  }

  auto iter_member     = varRefElement.begin();
  auto iter_member_end = varRefElement.end();
  for(; iter_member != iter_member_end; ++iter_member)
  {
    if(iter_member.IsArray())
    {
      auto iter_sub_element = iter_member.array_begin();
      auto iter_sub_element_end = iter_member.array_end();
      for(int n = 0; iter_sub_element != iter_sub_element_end; ++iter_sub_element, n++)
      {
        str.Format("%s.%s[%d]", szVarFormat, iter_member.VariableName(), n);
        TestSlidingArray(pDataPool, varArray, str.CStr());
      }
    }
    else
    {
      str.Format("%s.%s", szVarFormat, iter_member.VariableName());
      TestSlidingArray(pDataPool, varArray, str.CStr());
    }
  }

}

//void TestSlidingArray(DataPoolVariable& varArray)
//{
//  ASSERT(varArray.GetCaps() & Marimo::DataPoolVariable::CAPS_ARRAY); // 必须是数组
//  clStringA strName = varArray.GetName();
//  DataPoolVariable varElement = varArray[0];
//  size_t val;
//  for(GXSIZE_T i = 0; i < varArray.GetLength(); i++)
//  {
//    DataPoolVariable varRefElement = varArray[i];
//    CHECK_VALUE(val, varRefElement.GetBuffer() - varElement.GetBuffer(), 0);
//    CHECK_VALUE(val, varRefElement.GetOffset() - varElement.GetOffset(), 0);
//    CHECK_VALUE(val, (size_t)varRefElement.GetPtr() - (size_t)varElement.GetPtr(), 0);
//    CHECK_VALUE(val, varRefElement.GetSize() - varElement.GetSize(), 0);
//    varArray.Sliding(&varElement, 1);
//  }
//
//  varElement = varArray[0];
//  auto iter_member     = varElement.begin();
//  auto iter_member_end = varElement.end();
//  for(; iter_member != iter_member_end; ++iter_member)
//  {
//    varElement = varArray[0][iter_member.VariableName()];
//    for(GXSIZE_T i = 0; i < varArray.GetLength(); i++)
//    {
//      DataPoolVariable varRefElementMember = varArray[i][iter_member.VariableName()];
//
//      TestSlidingElementMember(varArray, iter_member.ToVariable());
//    }
//  }
//
//  //for(auto iter_element = VarArray.array_begin(); iter_element != VarArray.array_end(); ++iter_element)
//  //{
//  //}
//}

//////////////////////////////////////////////////////////////////////////

void TestShaderConstantBuffer_Common()
{
  static Marimo::DATAPOOL_VARIABLE_DECLARATION s_aShaderConstantBuffer[] =
  {
    {"float4x4", "WorldViewProjection", 0,},
    {"float4",   "DiffuseColor", 0,},
    {"float3",   "LightPosition", 0, 4},
    {"float3",   "EyePosition"},
    {"float3",   "EyeDirection"},
    {NULL, NULL}
  };

  Marimo::DataPool* pDataPool = NULL;
  GXHRESULT hr = Marimo::DataPool::CreateDataPool(&pDataPool, NULL, NULL, s_aShaderConstantBuffer, Marimo::DataPoolCreation_NotCross16BytesBoundary);
  if(GXSUCCEEDED(hr)) {
    pDataPool->Save(_CLTEXT("Test\\TestShaderConstantBuffer.dpl"));
  }

  MOVarMatrix4 matWVProj;
  MOVarFloat4  vDiffuseColor;
  Marimo::DataPoolVariable vLightPositionArray;
  Marimo::DataPoolVariable  vLightPosition[4];
  MOVarFloat3  vEyePosition;
  MOVarFloat3  vEyeDirection;

  GXBOOL bRetult;
  bRetult = pDataPool->QueryByName("WorldViewProjection", &matWVProj);
  ASSERT(bRetult);
  ASSERT(matWVProj.GetOffset() == 0);

  bRetult = pDataPool->QueryByName("DiffuseColor", &vDiffuseColor);
  ASSERT(bRetult);
  ASSERT(vDiffuseColor.GetOffset() == sizeof(float4x4));

  bRetult = pDataPool->QueryByName("LightPosition", &vLightPositionArray);
  ASSERT(bRetult);
  ASSERT(TEST_FLAG(vLightPositionArray.GetCaps(), Marimo::DataPoolVariable::CAPS_ARRAY));
  ASSERT(vLightPositionArray.GetOffset() == sizeof(float4x4) + sizeof(float4));
  ASSERT(vLightPositionArray.GetSize() == sizeof(float4) * 3 + sizeof(float3));
  vLightPosition[0] = vLightPositionArray[0];
  vLightPosition[1] = vLightPositionArray[1];
  vLightPosition[2] = vLightPositionArray[2];
  vLightPosition[3] = vLightPositionArray[3];
  ASSERT(vLightPosition[0].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float4) * 0);
  ASSERT(vLightPosition[1].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float4) * 1);
  ASSERT(vLightPosition[2].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float4) * 2);
  ASSERT(vLightPosition[3].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float4) * 3);

  bRetult = pDataPool->QueryByName("EyePosition", &vEyePosition);
  ASSERT(bRetult);
  ASSERT(vEyePosition.GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float4) * 4);

  bRetult = pDataPool->QueryByName("EyeDirection", &vEyeDirection);
  ASSERT(bRetult);
  ASSERT(vEyeDirection.GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float4) * 4 + sizeof(float4));

  SAFE_RELEASE(pDataPool);
}

void TestShaderConstantBuffer_HLSLPackingRulesSample()
{
  Marimo::DataPool* pDataPool = NULL;
  GXHRESULT hr = Marimo::DataPool::CompileFromMemory(&pDataPool, NULL, NULL, g_szCodeSample, 0, Marimo::DataPoolCreation_NotCross16BytesBoundary);

  Marimo::DataPoolVariable ie1, ie2, ie3, ie4, ie5, ie6, ie7, ie8, ie9, ie10, ie11, ie12;
  pDataPool->QueryByName("ie1", &ie1);
  pDataPool->QueryByName("ie2", &ie2);
  pDataPool->QueryByName("ie3", &ie3);
  pDataPool->QueryByName("ie4", &ie4);
  pDataPool->QueryByName("ie5", &ie5);
  pDataPool->QueryByName("ie6", &ie6);
  pDataPool->QueryByName("ie7", &ie7);
  pDataPool->QueryByName("ie8", &ie8);
  pDataPool->QueryByName("ie9", &ie9);
  pDataPool->QueryByName("ie10", &ie10);
  pDataPool->QueryByName("ie11", &ie11);
  pDataPool->QueryByName("ie12", &ie12);

  GXUINT n; // 用于失败时查看值

  CHECK_VALUE(n, ie1.MemberOf("Val1").GetOffset(), 0);
  CHECK_VALUE(n, ie1.MemberOf("Val2").GetOffset(), 16);
  CHECK_VALUE(n, ie1.MemberOf("Val3").GetOffset(), 24);

  CHECK_VALUE(n, ie2.MemberOf("Val1").GetOffset() - ie2.GetOffset(), 0);
  CHECK_VALUE(n, ie2.MemberOf("Val2").GetOffset() - ie2.GetOffset(), 16);
  CHECK_VALUE(n, ie2.MemberOf("Val3").GetOffset() - ie2.GetOffset(), 32);

  CHECK_VALUE(n, ie3.MemberOf("Val1").GetOffset() - ie3.GetOffset(), 0);
  CHECK_VALUE(n, ie3.MemberOf("Val2").GetOffset() - ie3.GetOffset(), 4);
  CHECK_VALUE(n, ie3.MemberOf("Val3").GetOffset() - ie3.GetOffset(), 8);

  CHECK_VALUE(n, ie4.MemberOf("Val1").GetOffset() - ie4.GetOffset(), 0);
  CHECK_VALUE(n, ie4.MemberOf("Val2").GetOffset() - ie4.GetOffset(), 4);
  CHECK_VALUE(n, ie4.MemberOf("Val3").GetOffset() - ie4.GetOffset(), 12);

  CHECK_VALUE(n, ie5.MemberOf("Val1").GetOffset() - ie5.GetOffset(), 0);
  CHECK_VALUE(n, ie5.MemberOf("Val1_1").GetOffset() - ie5.GetOffset(), 4);
  CHECK_VALUE(n, ie5.MemberOf("Val1_2").GetOffset() - ie5.GetOffset(), 8);
  CHECK_VALUE(n, ie5.MemberOf("Val2").GetOffset() - ie5.GetOffset(), 16);

  CHECK_VALUE(n, ie6.MemberOf("Val1").GetOffset() - ie6.GetOffset(), 0);
  CHECK_VALUE(n, ie6.MemberOf("Val2").GetOffset() - ie6.GetOffset(), 12);

  CHECK_VALUE(n, ie7.MemberOf("Val1").GetOffset() - ie7.GetOffset(), 0);
  CHECK_VALUE(n, ie7.MemberOf("Val2").GetOffset() - ie7.GetOffset(), 4);

  CHECK_VALUE(n, ie8.MemberOf("Val1").GetOffset() - ie8.GetOffset(), 0);
  CHECK_VALUE(n, ie8.MemberOf("Val1_1").GetOffset() - ie8.GetOffset(), 4);
  CHECK_VALUE(n, ie8.MemberOf("Val2").GetOffset() - ie8.GetOffset(), 16);

  CHECK_VALUE(n, ie9.MemberOf("Val1").GetOffset() - ie9.GetOffset(), 0);
  CHECK_VALUE(n, ie9.MemberOf("Val2").MemberOf("SVal1").GetOffset() - ie9.GetOffset(), 16);
  CHECK_VALUE(n, ie9.MemberOf("Val2").MemberOf("SVal2").GetOffset() - ie9.GetOffset(), 32);

  CHECK_VALUE(n, ie10.MemberOf("Val1").GetOffset() - ie10.GetOffset(), 0);
  CHECK_VALUE(n, ie10.MemberOf("Val2").MemberOf("SVal1").GetOffset() - ie10.GetOffset(), 16);
  CHECK_VALUE(n, ie10.MemberOf("Val2").MemberOf("SVal2").GetOffset() - ie10.GetOffset(), 32);

  CHECK_VALUE(n, ie11.MemberOf("Val1").MemberOf("SVal1").GetOffset() - ie11.GetOffset(), 0);
  CHECK_VALUE(n, ie11.MemberOf("Val1").MemberOf("SVal2").GetOffset() - ie11.GetOffset(), 16);
  CHECK_VALUE(n, ie11.MemberOf("Val2").GetOffset() - ie11.GetOffset(), 32);

  CHECK_VALUE(n, ie12.MemberOf("Val1").MemberOf("SVal1").GetOffset() - ie12.GetOffset(), 0);
  CHECK_VALUE(n, ie12.MemberOf("Val1").MemberOf("SVal2").GetOffset() - ie12.GetOffset(), 16);
  CHECK_VALUE(n, ie12.MemberOf("Val1").MemberOf("SVal3").GetOffset() - ie12.GetOffset(), 24);
  CHECK_VALUE(n, ie12.MemberOf("Val1").MemberOf("SVal4").GetOffset() - ie12.GetOffset(), 32);
  CHECK_VALUE(n, ie12.MemberOf("Val2").GetOffset() - ie12.GetOffset(), 48);
  CHECK_VALUE(n, ie12.MemberOf("Val3").GetOffset() - ie12.GetOffset(), 64);
  CHECK_VALUE(n, ie12.MemberOf("Val4").GetOffset() - ie12.GetOffset(), 80);
  CHECK_VALUE(n, ie12.MemberOf("Val5").GetOffset() - ie12.GetOffset(), 96);

  SAFE_RELEASE(pDataPool);
}

//////////////////////////////////////////////////////////////////////////

void CheckCommon1(const Marimo::DataPoolVariable& a1,
  const Marimo::DataPoolVariable& a2,
  const Marimo::DataPoolVariable& mat4x3,
  const Marimo::DataPoolVariable& mat3x4,
  const Marimo::DataPoolVariable& mats,
  Marimo::DataPoolUtility::iterator& iter_var)
{
  GXUINT val; // 用于失败时查看值
  const GXUINT nBaseOffset = a1.GetOffset();

  CHECK_VALUE(val, a1.GetOffset()     - nBaseOffset, 0);
  CHECK_VALUE(val, a2.GetOffset()     - nBaseOffset, 16 * 7);
  CHECK_VALUE(val, mat4x3.GetOffset() - nBaseOffset, 224);
  CHECK_VALUE(val, mat3x4.GetOffset() - nBaseOffset, 272);
  CHECK_VALUE(val, mats.GetOffset()   - nBaseOffset, 336);

  CHECK_VALUE(val, a1.GetSize(), 100);
  CHECK_VALUE(val, a1.GetLength(), 7);
  CHECK_VALUE(val, a1.IndexOf(0).GetOffset() - nBaseOffset, 0);
  CHECK_VALUE(val, a1.IndexOf(1).GetOffset() - nBaseOffset, 16 * 1);
  CHECK_VALUE(val, a1.IndexOf(2).GetOffset() - nBaseOffset, 16 * 2);
  CHECK_VALUE(val, a1.IndexOf(3).GetOffset() - nBaseOffset, 16 * 3);
  CHECK_VALUE(val, a1.IndexOf(4).GetOffset() - nBaseOffset, 16 * 4);
  CHECK_VALUE(val, a1.IndexOf(5).GetOffset() - nBaseOffset, 16 * 5);
  CHECK_VALUE(val, a1.IndexOf(6).GetOffset() - nBaseOffset, 16 * 6);

  CHECK_VALUE(val, a2.GetSize(), 104);
  CHECK_VALUE(val, a2.GetLength(), 7);
  CHECK_VALUE(val, a2.IndexOf(0).GetOffset() - a2.GetOffset(), 0);
  CHECK_VALUE(val, a2.IndexOf(1).GetOffset() - a2.GetOffset(), 16 * 1);
  CHECK_VALUE(val, a2.IndexOf(2).GetOffset() - a2.GetOffset(), 16 * 2);
  CHECK_VALUE(val, a2.IndexOf(3).GetOffset() - a2.GetOffset(), 16 * 3);
  CHECK_VALUE(val, a2.IndexOf(4).GetOffset() - a2.GetOffset(), 16 * 4);
  CHECK_VALUE(val, a2.IndexOf(5).GetOffset() - a2.GetOffset(), 16 * 5);
  CHECK_VALUE(val, a2.IndexOf(6).GetOffset() - a2.GetOffset(), 16 * 6);

  CHECK_VALUE(val, mat4x3.GetSize(), 48);
  CHECK_VALUE(val, mat4x3.GetLength(), 1);
  CHECK_VALUE(val, mat4x3.MemberOf("v").GetSize(), 48);
  CHECK_VALUE(val, mat4x3.MemberOf("v").GetLength(), 3);
  CHECK_VALUE(val, mat4x3.MemberOf("v").GetOffset() - mat4x3.GetOffset(), 0);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(0).GetOffset() - mat4x3.GetOffset(), 0);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(1).GetOffset() - mat4x3.GetOffset(), 16);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(2).GetOffset() - mat4x3.GetOffset(), 32);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(0).GetSize(), 16);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(1).GetSize(), 16);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(2).GetSize(), 16);
  CHECK_VALUE(val, mat4x3.MemberOf("v").IndexOf(3).IsValid(), 0);

  CHECK_VALUE(val, mat3x4.GetSize(), 60);
  CHECK_VALUE(val, mat3x4.GetLength(), 1);
  CHECK_VALUE(val, mat3x4.MemberOf("v").GetSize(), 60);
  CHECK_VALUE(val, mat3x4.MemberOf("v").GetLength(), 4);
  CHECK_VALUE(val, mat3x4.MemberOf("v").GetOffset() - mat3x4.GetOffset(), 0);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(0).GetOffset() - mat3x4.GetOffset(), 0);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(1).GetOffset() - mat3x4.GetOffset(), 16);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(2).GetOffset() - mat3x4.GetOffset(), 32);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(0).GetSize(), 12);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(1).GetSize(), 12);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(2).GetSize(), 12);
  CHECK_VALUE(val, mat3x4.MemberOf("v").IndexOf(3).GetSize(), 12);

  CHECK_VALUE(val, mats.GetSize(), 444);
  CHECK_VALUE(val, mats.GetLength(), 7);
  CHECK_VALUE(val, mats.IndexOf(3).GetSize(), 60);
  CHECK_VALUE(val, mats.IndexOf(3).MemberOf("v").GetLength(), 4);
  CHECK_VALUE(val, mats.IndexOf(3).MemberOf("v").GetOffset() - mats.GetOffset(), 64 * 3);
  CHECK_VALUE(val, mats.IndexOf(3).MemberOf("v").IndexOf(0).GetOffset() - mats.GetOffset(), 64 * 3);
  CHECK_VALUE(val, mats.IndexOf(3).MemberOf("v").IndexOf(1).GetOffset() - mats.GetOffset(), 64 * 3 + 16);
  CHECK_VALUE(val, mats.IndexOf(3).MemberOf("v").IndexOf(2).GetOffset() - mats.GetOffset(), 64 * 3 + 32);

  Marimo::DataPoolVariable var;
  //auto iter_var = pDataPool->begin();
  //
  // a1
  //
  CHECK_VALUE(val, iter_var.offset() - nBaseOffset, 0);

#if 1
  auto iter_ele = iter_var.array_begin();
  for(int i = 0; i < 7; i++)
  {
    var = iter_ele.ToVariable();
    CHECK_VALUE(val, var.GetOffset() - nBaseOffset, i * 16);
    ++iter_ele;
  }
#endif

  ++iter_var;
  //
  // a2
  //
  CHECK_VALUE(val, iter_var.offset() - nBaseOffset, 16 * 7);
  iter_ele = iter_var.array_begin();
#if 1
  for(int i = 0; i < 7; i++)
  {
    var = iter_ele.ToVariable();
    CHECK_VALUE(val, var.GetOffset() - iter_var.offset(), i * 16);
    ++iter_ele;
  }
#endif

  ++iter_var;
  //
  // mat4x3
  //
  CHECK_VALUE(val, iter_var.offset() - nBaseOffset, 224);
  auto iter_var_mat4x3 = iter_var.begin();  // mat4x3.v
  iter_ele = iter_var_mat4x3.array_begin(); // mat4x3.v[0]
  for(int i = 0; i < 3; i++)
  {
    var = iter_ele.ToVariable();
    CHECK_VALUE(val, var.GetOffset() - iter_var.offset(), i * 16);
    ++iter_ele;
  }


  ++iter_var;
  //
  // mat3x4
  //
  CHECK_VALUE(val, iter_var.offset() - nBaseOffset, 272);
  auto iter_var_mat3x4 = iter_var.begin();  // mat3x4.v
  iter_ele = iter_var_mat3x4.array_begin(); // mat3x4.v[0]
  for(int i = 0; i < 3; i++)
  {
    var = iter_ele.ToVariable();
    CHECK_VALUE(val, var.GetOffset() - iter_var.offset(), i * 16);
    ++iter_ele;
  }

  ++iter_var;
  //
  // mats
  //
  CHECK_VALUE(val, iter_var.offset() - nBaseOffset, 336);
  auto iter_mats_ele = mats.array_begin(); // mat[0]
  for(int i = 0; i < 7; i++)
  {
    iter_ele = iter_mats_ele.begin().array_begin(); // mat[0].v[0]
    for(int k = 0; k < 3; k++)
    {
      var = iter_ele.ToVariable();
      CHECK_VALUE(val, var.GetOffset() - mats[i]["v"].GetOffset(), k * 16);
      ++iter_ele;
    }
    ++iter_mats_ele;
  }
}

void TestShaderConstantBuffer_HLSLPackingRulesSample2()
{
  static GXLPCSTR szCodeSample2 = "\
  float  a1[7];     \n\
  float2 a2[7];     \n\
  float4x3 mat4x3;  \n\
  float3x4 mat3x4;  \n\
  float3x4 mats[7]; \n\
";

  Marimo::DataPool* pDataPool = NULL;
  GXHRESULT hr = Marimo::DataPool::CompileFromMemory(&pDataPool, NULL, NULL, szCodeSample2, 0, Marimo::DataPoolCreation_NotCross16BytesBoundary);
  ASSERT(GXSUCCEEDED(hr));

  Marimo::DataPoolVariable a1, a2, mat4x3, mat3x4, mats;
  pDataPool->QueryByName("a1", &a1);         // [100] (0, 112)
  pDataPool->QueryByName("a2", &a2);         // [104] (112, 224)
  pDataPool->QueryByName("mat4x3", &mat4x3); // [48]  (224, 272)
  pDataPool->QueryByName("mat3x4", &mat3x4); // [60]  (272, 336)
  pDataPool->QueryByName("mats", &mats);     // [444] (336, 776)

  GXUINT val;
  CHECK_VALUE(val, a1.GetOffset(), 0);
  CHECK_VALUE(val, a2.GetOffset(), 16 * 7);
  CHECK_VALUE(val, mat4x3.GetOffset(), 224);
  CHECK_VALUE(val, mat3x4.GetOffset(), 272);
  CHECK_VALUE(val, mats.GetOffset(), 336);

  // 测试 QueryByExpression()
  {
    Marimo::DataPoolVariable a1_2, a2_2, mats_2;
    pDataPool->QueryByExpression("a1[2]", &a1_2);
    pDataPool->QueryByExpression("a2[2]", &a2_2);
    pDataPool->QueryByExpression("mats[2]", &mats_2);
    CHECK_VALUE(val, a1_2.GetOffset() - a1.GetOffset(), 16 * 2);
    CHECK_VALUE(val, a2_2.GetOffset() - a2.GetOffset(), 16 * 2);
    CHECK_VALUE(val, mats_2.GetOffset() - mats.GetOffset(), 64 * 2);
  }

  CheckCommon1(a1, a2, mat4x3, mat3x4, mats, pDataPool->begin());

  EnumerateVariables(pDataPool);
  EnumeratePtrControl(pDataPool);

  TestSlidingArray(pDataPool, a1, "a1[%d]");
  TestSlidingArray(pDataPool, a2, "a2[%d]");
  TestSlidingArray(pDataPool, mats, "mats[%d]");

  SAFE_RELEASE(pDataPool);
}

void TestShaderConstantBuffer_HLSLPackingRulesSample3()
{
  static GXLPCSTR szCodeSample3 = "\
struct STU {          \n\
  float  a1[7];       \n\
  float2 a2[7];       \n\
  float4x3 mat4x3;    \n\
  float3x4 mat3x4;    \n\
  float3x4 mats[7];   \n\
};                    \n\
STU stu_array[];      \n\
float3x4 mat_array[]; \n\
float2 vc2_array[];   \n\
float  val_array[];   \n\
";

  Marimo::DataPool* pDataPool = NULL;
  GXHRESULT hr = Marimo::DataPool::CompileFromMemory(&pDataPool, NULL, NULL, szCodeSample3, 0, Marimo::DataPoolCreation_NotCross16BytesBoundary);
  ASSERT(GXSUCCEEDED(hr));

  Marimo::DataPoolVariable stu_array, mat_array, vc2_array, val_array;
  Marimo::DataPoolVariable new_ele;
  GXUINT val;

  pDataPool->QueryByName("stu_array", &stu_array);
  pDataPool->QueryByName("mat_array", &mat_array);
  pDataPool->QueryByName("vc2_array", &vc2_array);
  pDataPool->QueryByName("val_array", &val_array);

  CHECK_VALUE(val, stu_array.GetOffset(), sizeof(void*) * 0);
  CHECK_VALUE(val, mat_array.GetOffset(), sizeof(void*) * 1);
  CHECK_VALUE(val, vc2_array.GetOffset(), sizeof(void*) * 2);
  CHECK_VALUE(val, val_array.GetOffset(), sizeof(void*) * 3);

  CHECK_VALUE(val, stu_array.GetSize(), 0);
  CHECK_VALUE(val, mat_array.GetSize(), 0);
  CHECK_VALUE(val, vc2_array.GetSize(), 0);
  CHECK_VALUE(val, val_array.GetSize(), 0);

  CHECK_VALUE(val, stu_array.GetLength(), 0);
  CHECK_VALUE(val, mat_array.GetLength(), 0);
  CHECK_VALUE(val, vc2_array.GetLength(), 0);
  CHECK_VALUE(val, val_array.GetLength(), 0);


  for(int i = 0; i < 7; i++)
  {
    new_ele = mat_array.NewBack();
    CHECK_VALUE(val, mat_array.GetSize(), i * 64 + 60);
    CHECK_VALUE(val, mat_array.GetLength(), i + 1);

    CHECK_VALUE(val, new_ele.GetSize(), 60);
     CHECK_VALUE(val, new_ele.GetOffset(), 64 * i);
  }

  for(int i = 0; i < 5; i++)
  {
    new_ele = vc2_array.NewBack();
    CHECK_VALUE(val, vc2_array.GetSize(), i * 16 + sizeof(float2));
    CHECK_VALUE(val, vc2_array.GetLength(), i + 1);

    CHECK_VALUE(val, new_ele.GetSize(), 8);
    CHECK_VALUE(val, new_ele.GetOffset(), 16 * i);
  }

  for(int i = 0; i < 9; i++)
  {
    new_ele = val_array.NewBack();
    CHECK_VALUE(val, val_array.GetSize(), i * 16 + sizeof(float));
    CHECK_VALUE(val, val_array.GetLength(), i + 1);

    CHECK_VALUE(val, new_ele.GetSize(), 4);
    CHECK_VALUE(val, new_ele.GetOffset(), 16 * i);
  }

  for(int i = 0; i < 5; i++)
  {
    new_ele = stu_array.NewBack();
    // 结构体长度是按照16字节对齐的
    CHECK_VALUE(val, stu_array.GetSize(), (i + 1) * 784);
    CHECK_VALUE(val, stu_array.GetLength(), i + 1);

    CHECK_VALUE(val, new_ele.GetSize(), 784);
    CHECK_VALUE(val, new_ele.GetOffset(), 784 * i);
    CheckCommon1(new_ele["a1"], new_ele["a2"], new_ele["mat4x3"], new_ele["mat3x4"], new_ele["mats"], new_ele.begin());
  }

  EnumerateVariables(pDataPool);
  EnumeratePtrControl(pDataPool);

  TestSlidingArray(pDataPool, stu_array, "stu_array[%d]");
  TestSlidingArray(pDataPool, mat_array, "mat_array[%d]");
  TestSlidingArray(pDataPool, vc2_array, "vc2_array[%d]");
  TestSlidingArray(pDataPool, val_array, "val_array[%d]");


  for(auto iter_ele = stu_array.array_begin(); iter_ele != stu_array.array_end(); ++iter_ele)
  {
    Marimo::DataPoolVariable var_stu = iter_ele.ToVariable();
    CHECK_VALUE(val, var_stu.GetSize(), 784);
    CHECK_VALUE(val, var_stu.GetOffset() % 784, 0);
    int i = var_stu.GetOffset() / 784;

    auto iter_member = iter_ele.begin();
    Marimo::DataPoolVariable var_a1 = iter_member.ToVariable();
    Marimo::DataPoolVariable var_a2 = (++iter_member).ToVariable();
    Marimo::DataPoolVariable var_mat4x3 = (++iter_member).ToVariable();
    Marimo::DataPoolVariable var_mat3x4 = (++iter_member).ToVariable();
    Marimo::DataPoolVariable var_mats = (++iter_member).ToVariable();
    CheckCommon1(var_a1, var_a2, var_mat4x3, var_mat3x4, var_mats, iter_ele.begin());
  }

  // 测试QueryByExpression()
  {
    Marimo::DataPoolVariable stu_array_2, vc2_array_2, val_array_2;
    pDataPool->QueryByExpression("stu_array[2]", &stu_array_2);
    pDataPool->QueryByExpression("vc2_array[2]", &vc2_array_2);
    pDataPool->QueryByExpression("val_array[2]", &val_array_2);
    CHECK_VALUE(val, stu_array_2.GetOffset() - stu_array[0].GetOffset(), 784 * 2);
    CHECK_VALUE(val, vc2_array_2.GetOffset() - vc2_array[0].GetOffset(), 16 * 2);
    CHECK_VALUE(val, val_array_2.GetOffset() - val_array[0].GetOffset(), 16 * 2);
  }

  {
    static GXUINT aSampleLen[] = { 5,7,5,9 };
    int i = 0;
    for(auto iter_vars = pDataPool->begin(); iter_vars != pDataPool->end(); ++iter_vars, i++)
    {
      CHECK_VALUE(val, iter_vars.array_length(), aSampleLen[i]);
    }
  }

  // 测试删除
  {
    GXUINT len;
    len = stu_array.GetLength() - 1;
    stu_array.Remove(len);
    CHECK_VALUE(val, stu_array.GetSize(), 784 * len);

    len = mat_array.GetLength() - 1;
    mat_array.Remove(len);
    CHECK_VALUE(val, mat_array.GetSize(), len * 64 - 4);

    len = vc2_array.GetLength() - 1;
    vc2_array.Remove(len);
    CHECK_VALUE(val, vc2_array.GetSize(), len * 16 - 8);

    len = val_array.GetLength() - 1;
    val_array.Remove(len);
    CHECK_VALUE(val, val_array.GetSize(), len * 16 - 12);
  }


  SAFE_RELEASE(pDataPool);
}

//////////////////////////////////////////////////////////////////////////

void TestShaderConstantBuffer()
{
  TestShaderConstantBuffer_Common();
  TestShaderConstantBuffer_HLSLPackingRulesSample();
  TestShaderConstantBuffer_HLSLPackingRulesSample2();
  TestShaderConstantBuffer_HLSLPackingRulesSample3();
}