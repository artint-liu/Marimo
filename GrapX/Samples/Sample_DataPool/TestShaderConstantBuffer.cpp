#include <tchar.h>
#include <Marimo.H>
#include "GrapX/DataPool.H"
#include <clTextLines.h>
#include <clString.H>
#include "clPathFile.h"
#include <vld.h>

// Shader constant buffer打包规则参考：
// https://docs.microsoft.com/zh-cn/windows/desktop/direct3dhlsl/dx-graphics-hlsl-packing-rules

#define __STR__(_V) #_V
#define __STR2__(_V) __STR__(_V)
#define __LINE_STR__ __STR2__(__LINE__)

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

void TestShaderConstantBuffer_Common()
{
  static Marimo::VARIABLE_DECLARATION s_aShaderConstantBuffer[] =
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
    pDataPool->SaveW(_CLTEXT("Test\\TestShaderConstantBuffer.dpl"));
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
  ASSERT(vLightPositionArray.GetSize() == sizeof(float3) * 4);
  vLightPosition[0] = vLightPositionArray[0];
  vLightPosition[1] = vLightPositionArray[1];
  vLightPosition[2] = vLightPositionArray[2];
  vLightPosition[3] = vLightPositionArray[3];
  ASSERT(vLightPosition[0].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float3) * 0);
  ASSERT(vLightPosition[1].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float3) * 1);
  ASSERT(vLightPosition[2].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float3) * 2);
  ASSERT(vLightPosition[3].GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float3) * 3);

  bRetult = pDataPool->QueryByName("EyePosition", &vEyePosition);
  ASSERT(bRetult);
  ASSERT(vEyePosition.GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float3) * 4);

  bRetult = pDataPool->QueryByName("EyeDirection", &vEyeDirection);
  ASSERT(bRetult);
  ASSERT(vEyeDirection.GetOffset() == sizeof(float4x4) + sizeof(float4) + sizeof(float3) * 4 + sizeof(float4));

  SAFE_RELEASE(pDataPool);
}

#define CHECK_OFFSET(_VAR, _EXPR, _VAL) ASSERT((_VAR = _EXPR) == _VAL)
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

  GXUINT n;

  CHECK_OFFSET(n, ie1.MemberOf("Val1").GetOffset(), 0);
  CHECK_OFFSET(n, ie1.MemberOf("Val2").GetOffset(), 16);
  CHECK_OFFSET(n, ie1.MemberOf("Val3").GetOffset(), 24);

  CHECK_OFFSET(n, ie2.MemberOf("Val1").GetOffset() - ie2.GetOffset(), 0);
  CHECK_OFFSET(n, ie2.MemberOf("Val2").GetOffset() - ie2.GetOffset(), 16);
  CHECK_OFFSET(n, ie2.MemberOf("Val3").GetOffset() - ie2.GetOffset(), 32);

  CHECK_OFFSET(n, ie3.MemberOf("Val1").GetOffset() - ie3.GetOffset(), 0);
  CHECK_OFFSET(n, ie3.MemberOf("Val2").GetOffset() - ie3.GetOffset(), 4);
  CHECK_OFFSET(n, ie3.MemberOf("Val3").GetOffset() - ie3.GetOffset(), 8);

  CHECK_OFFSET(n, ie4.MemberOf("Val1").GetOffset() - ie4.GetOffset(), 0);
  CHECK_OFFSET(n, ie4.MemberOf("Val2").GetOffset() - ie4.GetOffset(), 4);
  CHECK_OFFSET(n, ie4.MemberOf("Val3").GetOffset() - ie4.GetOffset(), 12);

  CHECK_OFFSET(n, ie5.MemberOf("Val1").GetOffset() - ie5.GetOffset(), 0);
  CHECK_OFFSET(n, ie5.MemberOf("Val1_1").GetOffset() - ie5.GetOffset(), 4);
  CHECK_OFFSET(n, ie5.MemberOf("Val1_2").GetOffset() - ie5.GetOffset(), 8);
  CHECK_OFFSET(n, ie5.MemberOf("Val2").GetOffset() - ie5.GetOffset(), 16);

  CHECK_OFFSET(n, ie6.MemberOf("Val1").GetOffset() - ie6.GetOffset(), 0);
  CHECK_OFFSET(n, ie6.MemberOf("Val2").GetOffset() - ie6.GetOffset(), 12);

  CHECK_OFFSET(n, ie7.MemberOf("Val1").GetOffset() - ie7.GetOffset(), 0);
  CHECK_OFFSET(n, ie7.MemberOf("Val2").GetOffset() - ie7.GetOffset(), 4);

  CHECK_OFFSET(n, ie8.MemberOf("Val1").GetOffset() - ie8.GetOffset(), 0);
  CHECK_OFFSET(n, ie8.MemberOf("Val1_1").GetOffset() - ie8.GetOffset(), 4);
  CHECK_OFFSET(n, ie8.MemberOf("Val2").GetOffset() - ie8.GetOffset(), 16);

  CHECK_OFFSET(n, ie9.MemberOf("Val1").GetOffset() - ie9.GetOffset(), 0);
  CHECK_OFFSET(n, ie9.MemberOf("Val2").MemberOf("SVal1").GetOffset() - ie9.GetOffset(), 16);
  CHECK_OFFSET(n, ie9.MemberOf("Val2").MemberOf("SVal2").GetOffset() - ie9.GetOffset(), 32);

  CHECK_OFFSET(n, ie10.MemberOf("Val1").GetOffset() - ie10.GetOffset(), 0);
  CHECK_OFFSET(n, ie10.MemberOf("Val2").MemberOf("SVal1").GetOffset() - ie10.GetOffset(), 16);
  CHECK_OFFSET(n, ie10.MemberOf("Val2").MemberOf("SVal2").GetOffset() - ie10.GetOffset(), 32);

  CHECK_OFFSET(n, ie11.MemberOf("Val1").MemberOf("SVal1").GetOffset() - ie11.GetOffset(), 0);
  CHECK_OFFSET(n, ie11.MemberOf("Val1").MemberOf("SVal2").GetOffset() - ie11.GetOffset(), 16);
  CHECK_OFFSET(n, ie11.MemberOf("Val2").GetOffset() - ie11.GetOffset(), 32);

  CHECK_OFFSET(n, ie12.MemberOf("Val1").MemberOf("SVal1").GetOffset() - ie12.GetOffset(), 0);
  CHECK_OFFSET(n, ie12.MemberOf("Val1").MemberOf("SVal2").GetOffset() - ie12.GetOffset(), 16);
  CHECK_OFFSET(n, ie12.MemberOf("Val1").MemberOf("SVal3").GetOffset() - ie12.GetOffset(), 24);
  CHECK_OFFSET(n, ie12.MemberOf("Val1").MemberOf("SVal4").GetOffset() - ie12.GetOffset(), 32);
  CHECK_OFFSET(n, ie12.MemberOf("Val2").GetOffset() - ie12.GetOffset(), 48);
  CHECK_OFFSET(n, ie12.MemberOf("Val3").GetOffset() - ie12.GetOffset(), 64);
  CHECK_OFFSET(n, ie12.MemberOf("Val4").GetOffset() - ie12.GetOffset(), 80);
  CHECK_OFFSET(n, ie12.MemberOf("Val5").GetOffset() - ie12.GetOffset(), 96);

  SAFE_RELEASE(pDataPool);
}

void TestShaderConstantBuffer()
{
  TestShaderConstantBuffer_Common();
  TestShaderConstantBuffer_HLSLPackingRulesSample();
}