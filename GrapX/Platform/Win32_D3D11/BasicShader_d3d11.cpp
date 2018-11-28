#define __STR__(_V) #_V
#define __STR2__(_V) __STR__(_V)
#define __LINE_STR__ __STR2__(__LINE__)
namespace GrapX
{
  namespace D3D11
  {
    const char* g_szBaseShader = "#line " __LINE_STR__ " \"" __FILE__ "\"\n\
struct VS_INPUT                                                         \n\
{                                                                       \n\
  float4  Pos : POSITION;                                               \n\
  float4  Color : COLOR0;                                               \n\
  float2  TexUV : TEXCOORD0;                                            \n\
};                                                                      \n\
                                                                        \n\
struct PS_INPUT                                                         \n\
{                                                                       \n\
  float4  Pos : SV_POSITION;                                            \n\
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
Texture2D texSimple : register(t0);                                     \n\
SamplerState Simple_Sampler : register(s0);// = sampler_state           \n\
//{                                                                     \n\
//  Texture = <texSimple>;                                              \n\
//MagFilter = Linear;                                                   \n\
//MinFilter = Linear;                                                   \n\
//MipFilter = Linear;                                                   \n\
//AddressU = Clamp;                                                     \n\
//AddressV = Clamp;                                                     \n\
//};                                                                    \n\
                                                                        \n\
                                                                        \n\
float4 ps_main(PS_INPUT Input) : SV_Target                              \n\
{                                                                       \n\
  // 输入的纹理与顶点色相加                                              \n\
  float4 c = texSimple.Sample(Simple_Sampler, Input.TexUV);             \n\
  return (c * Color + ColorAdd) * Input.Color;                          \n\
}";

    const char* g_szFastGaussianBlur = "#line " __LINE_STR__ " \"" __FILE__ "\"\n\
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
sampler2D Simple_Sampler : register(s0);                                \n\
float fBlur;                                                            \n\
float3 draw(float2 uv) {                                                \n\
  return tex2D(Simple_Sampler, uv).rgb;                                 \n\
}                                                                       \n\
float rand(float2 co) {                                                 \n\
  return frac(sin(dot(co, float2(2.32, 5.85))) * 45333.23746) + 0.1;    \n\
}                                                                       \n\
float4 ps_main(PS_INPUT Input) : COLOR0                                 \n\
{                                                                       \n\
  float2 uv = Input.TexUV.xy;                                           \n\
  float bluramount = fBlur * 0.1;                                       \n\
  float3 blurred_image = 0;                                             \n\
  float sn, cn;                                                         \n\
                                                                        \n\
#define repeats 71.                                                     \n\
  for(float i = 0.; i < repeats; i++) {                                 \n\
                                                                        \n\
    sincos(degrees((i / repeats)*360.), sn, cn);                        \n\
    cn *= 1280.0 / 720.0;                                               \n\
    float2 q = float2(cn, sn) * (rand(float2(i, uv.x + uv.y)));         \n\
    float2 uv2 = uv + (q * bluramount);                                 \n\
    blurred_image += draw(uv2);                                         \n\
                                                                        \n\
  }                                                                     \n\
                                                                        \n\
  blurred_image /= repeats;                                             \n\
  return float4(blurred_image,1.0);                                     \n\
}                                                                       \n\
";
  } // namespace D3D9
} // namespace GrapX