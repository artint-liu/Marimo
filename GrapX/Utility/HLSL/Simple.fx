
#define PI          (3.1415926535897932384626433832795)

//
// Vertex Shader
//
#include "FXCommRegister.H"

struct VS_INPUT
{
  float4  Pos    : POSITION;
  float4  Color  : COLOR0;
  float2  TexUV  : TEXCOORD0;
};

struct PS_INPUT
{
  float4  Pos   : POSITION;
  float4  Color : COLOR0;
  float2  TexUV : TEXCOORD0;
};

PS_INPUT SimpleVS(VS_INPUT Input)
{
  PS_INPUT Output = (PS_INPUT)0;
  Output.Pos   = mul(matWVProj, Input.Pos);
  Output.TexUV = Input.TexUV;
  Output.Color = Input.Color;
  return Output;
}


//
// Pixel Shader
//
texture texSimple;
sampler2D Simple_Sampler : register(s0) = sampler_state
{
  Texture = <texSimple>;
  MagFilter = Linear;
  MinFilter = Linear;
  MipFilter = Linear;
  AddressU = Clamp;
  AddressV = Clamp;
};


float4 SimplePS(PS_INPUT Input) : COLOR0
{
  // 输入的纹理与顶点色相加
  float4 crPixel = tex2D(Simple_Sampler, Input.TexUV) + ColorAdd;
  return crPixel * Input.Color * Color;
}

//
// Tech
//
technique SimpleTech
{
    pass P0
    {
        VertexShader  = compile vs_2_0 SimpleVS();
        PixelShader   = compile ps_2_0 SimplePS();
    }
}