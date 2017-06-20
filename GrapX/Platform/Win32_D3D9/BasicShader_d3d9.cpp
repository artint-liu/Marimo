#define __STR__(_V) #_V
#define __STR2__(_V) __STR__(_V)
#define __LINE_STR__ __STR2__(__LINE__)

namespace D3D9
{
const char* g_szBaseShader = "#line "__LINE_STR__" \"" __FILE__ "\"\n\
struct VS_INPUT                                                         \n\
{                                                                       \n\
  float4  Pos : POSITION;                                               \n\
  float4  Color : COLOR0;                                               \n\
  float2  TexUV : TEXCOORD0;                                            \n\
};                                                                      \n\
                                                                        \n\
struct PS_INPUT                                                         \n\
{                                                                       \n\
  float4  Pos : POSITION;                                               \n\
  float4  Color : COLOR0;                                               \n\
  float2  TexUV : TEXCOORD0;                                            \n\
};                                                                      \n\
float4x4 matWVProj;                                                     \n\
float4 ColorAdd;                                                        \n\
float4 Color;                                                           \n\
PS_INPUT vs_main(VS_INPUT Input)                                        \n\
{                                                                       \n\
  PS_INPUT Output = (PS_INPUT)0;                                        \n\
  Output.Pos = mul(matWVProj, Input.Pos);                               \n\
  Output.TexUV = Input.TexUV;                                           \n\
  Output.Color = Input.Color;                                           \n\
  return Output;                                                        \n\
}                                                                       \n\
                                                                        \n\
                                                                        \n\
//                                                                      \n\
// Pixel Shader                                                         \n\
//                                                                      \n\
texture texSimple;                                                      \n\
sampler2D Simple_Sampler : register(s0) = sampler_state                 \n\
{                                                                       \n\
  Texture = <texSimple>;                                                \n\
MagFilter = Linear;                                                     \n\
MinFilter = Linear;                                                     \n\
MipFilter = Linear;                                                     \n\
AddressU = Clamp;                                                       \n\
AddressV = Clamp;                                                       \n\
};                                                                      \n\
                                                                        \n\
                                                                        \n\
float4 ps_main(PS_INPUT Input) : COLOR0                                 \n\
{                                                                       \n\
  // 输入的纹理与顶点色相加                                               \n\
  float4 crPixel = tex2D(Simple_Sampler, Input.TexUV) + ColorAdd;       \n\
  return crPixel * Input.Color * Color;                                 \n\
}";
} // namespace D3D9