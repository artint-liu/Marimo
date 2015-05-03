
#define PI          (3.1415926535897932384626433832795)

#include "FXCommRegister.H"
//
// Vertex Shader
//


void AeroVS(  float4  Pos    : POSITION,
        float2  TexUV  : TEXCOORD0,
      out float4  oPos  : POSITION,
      out  float2  oTexUV  : TEXCOORD0
      )
{
  oPos = mul(matWVProj, Pos);
  oTexUV = TexUV;
}












//
// Pixel Shader
//
texture texSimple;
sampler2D Frame_Sampler    : register(s0);
sampler2D Simple_Sampler  : register(s1);
sampler2D Noise_Sampler    : register(s2);

void AeroPS(  float4  iColor  : COLOR0,
        float2  TexUV  : TEXCOORD0,
        float2  ScreenPos : VPOS,
      out  float4  oColor  : COLOR0
  )
{
  float4 o = tex2D(Frame_Sampler, TexUV);
   float2 AeroUV = ScreenPos * TexelKernel * 0.5;
  // Ä£ºýÐ§¹û
  float4 c;
    c = (
      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x,-TexelKernel.y)) +//* 4 +
      tex2D(Simple_Sampler, AeroUV + float2(             0,-TexelKernel.y)) +//* 16 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x,-TexelKernel.y)) +//* 4 +
      
      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x,    0)) +//* 16 +
      tex2D(Simple_Sampler, AeroUV + float2(             0,    0)) +//* 25 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x,    0)) +//* 16 +

      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x, TexelKernel.y)) +//* 4 +
      tex2D(Simple_Sampler, AeroUV + float2(             0, TexelKernel.y)) +//* 16 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x, TexelKernel.y)) +//* 4 +

      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x * 2,-TexelKernel.y)) +//* 2 +
      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x * 2, TexelKernel.y)) +//* 2 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x * 2,-TexelKernel.y)) +//* 2 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x * 2, TexelKernel.y)) +//* 2 +

      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x,-TexelKernel.y * 2)) +//* 2 +
      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x, TexelKernel.y * 2)) +//* 2 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x,-TexelKernel.y * 2)) +//* 2 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x, TexelKernel.y * 2)) +//* 2 +

      tex2D(Simple_Sampler, AeroUV + float2(                 0,-TexelKernel.y * 2)) +//* 4 +
      tex2D(Simple_Sampler, AeroUV + float2(-TexelKernel.x * 2,                 0)) +//* 4 +
      tex2D(Simple_Sampler, AeroUV + float2( TexelKernel.x * 2,                 0)) +//* 4 +
      tex2D(Simple_Sampler, AeroUV + float2(                 0, TexelKernel.y * 2))  //* 4

      ) * (1.0 / 21.0);

  oColor.xyz = c.xyz * (1.0 - o.w) + o.xyz * o.w;
  oColor.w = 1.0;  
}

///////////////////////////////////////////////////////////////////////////////////////////////
void BlurVS(  float4  Pos    : POSITION,
        float2  TexUV  : TEXCOORD0,
        float4  iColor  : COLOR0,
      out float4  oPos  : POSITION,
      out  float2  oTexUV  : TEXCOORD0
    )
{
  oPos = mul(matWVProj, Pos);
  oTexUV = TexUV;
}


//
// Pixel Shader
//
//#define ENABLE_NOISE_BLUR
int radius = 6; 
void BlurPS(  float4  iColor  : COLOR0,
         float2  TexUV  : TEXCOORD0,
      out  float4  oColor  : COLOR0
    )
{
#ifdef ENABLE_NOISE_BLUR
#define SAMP_NUM 32
  float2 nuv = TexUV * 100;
  float4 c = 0;
  for(int i = 0; i < SAMP_NUM; i++)
  {
    float4 n = tex2D(Noise_Sampler, nuv) - 0.5f;
    c += tex2D(Frame_Sampler, TexUV + n * 0.002 * radius);
    nuv += 0.02f;
  }
  oColor = c / SAMP_NUM;
#else
   //float4 c = 0;
   //float k = 0;
   //for(int i = 0; i < radius; i++)
   //{
   //   for(int j = 0; j < radius; j++)
   //   {
   //      float2 f = float2(j - radius * 0.5, i - radius * 0.5);
   //      c += tex2D( Frame_Sampler, TexUV + f * TexelKernel);
   //      k += 1;
   //   }
   //}
   //oColor = c / k;
	oColor = float4(1,0,0,1);
#endif
}


//
// Tech
//
technique AeroTech
{
    pass P0
    {
        VertexShader  = compile vs_2_0 AeroVS();
        PixelShader    = compile ps_2_0 AeroPS();
    }
}